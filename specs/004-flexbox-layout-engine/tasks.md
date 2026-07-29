---

description: "Task list for Flexbox Layout Engine"

---

# Tasks: Flexbox Layout Engine

**Input**: Design documents from `specs/004-flexbox-layout-engine/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

## Format: `[ID] [P?] [Story] Description`

## Phase 1: BUILD Infrastructure

- [x] T001 Update `src/framework/layout/BUILD.bazel` with `cc_library` — `hdrs`, `srcs`, `includes = ["."]`, deps on `//src/framework/core` and `@yoga//:yoga`
- [x] T002 Add `cc_test` target `layout_test` to `tests/BUILD.bazel` — deps on `//src/framework/layout` and `@com_google_googletest//:gtest_main`

## Phase 2: Result Type & Header

- [x] T003 Create `src/framework/layout/layout_result.h` with `MeasureResult` struct (`Size size`, `Point position`)
- [x] T004 [P] Create `src/framework/layout/flex_layout.h` — `FlexLayout` class with tagged-parameter ctor, `SetChildren`, `Measure`, `Arrange`, private `YGNodeRef root_`, `vector<YGNodeRef> children_`
- [x] T005 [P] Define tag types in `flex_layout.h`: `Direction`, `JustifyContent`, `AlignItems`, `FlexWrap`, `Gap`, `Padding`, `Margin`

## Phase 3: Yoga Wrapping Implementation

- [x] T006 Implement `FlexLayout` constructor with C++17 fold expression — each `ProcessArg` calls `YGNodeStyleSet*`
- [x] T007 Implement `SetChildren` — stores child YGNodeRef references
- [x] T008 Implement `Measure` — `YGNodeStyleSetWidth/Height` on root, `YGNodeInsertChild` per child, `YGNodeCalculateLayout`, extract sizes via `YGNodeLayoutGetWidth/Height`
- [x] T009 Implement `Arrange` — read positions via `YGNodeLayoutGetLeft/Top`
- [x] T010 Implement destructor — `YGNodeFreeRecursive(root_)`

## Phase 4: Unit Tests

- [x] T011 Create `tests/layout_test.cc` — test: `Direction(kRow)` children lay out horizontally
- [x] T012 Add test: `Direction(kColumn)` — children lay out vertically
- [x] T013 Add test: `JustifyContent(kCenter)` — children centered in container
- [x] T014 Add test: `JustifyContent(kSpaceBetween)` — even spacing between children
- [x] T015 Add test: `AlignItems(kStretch)` — children stretched to cross-axis
- [x] T016 Add test: `Gap(8)` — correct spacing between adjacent children
- [x] T017 Add test: `Padding(12)` — children offset from container edge
- [x] T018 Add test: `Margin(8)` — margin outside children creates spacing
- [x] T019 Add test: `FlexWrap(kWrap)` — children wrap to next line when overflow
- [x] T020 Add test: `FlexGrow` / `FlexShrink` / `FlexBasis` — child with flex-grow takes remaining space; multiple grow children share proportionally; flex-shrink reduces size when container too small; flex-basis sets initial main-axis size
- [x] T021 Add test: `AlignContent(kCenter)` — multi-line content centered in cross-axis when flex-wrap is enabled
- [x] T022 Add test: edge cases — empty children list, flex-shrink, zero-size container — all handle gracefully without crash

## Phase 5: Public Header & Validation

- [ ] T023 Create `src/framework/public/include/native_ui/layout.h` — re-export `FlexLayout`, `MeasureResult`, and all tag types (`Direction`, `JustifyContent`, `AlignItems`, `FlexWrap`, `Gap`, `Padding`, `Margin`)
- [ ] T024 Update `src/framework/public/BUILD.bazel` — add `//src/framework/layout` to deps
- [ ] T025 Run full validation: `bazel build //...` + `bazel test //...`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (BUILD)**: No dependencies — can start immediately
- **Phase 2 (Header)**: Depends on Phase 1 (BUILD must exist)
- **Phase 3 (Impl)**: Depends on Phase 2 (header defines API)
- **Phase 4 (Tests)**: Depends on Phase 3 (implementation must compile)
- **Phase 5 (Public + BUILD)**: Depends on Phase 3 (header must exist)

### Parallel Opportunities

- T004-T005 both marked [P] — header and tags can be written concurrently
- T011-T022 tests can be added incrementally (each test is independent)
- T023 (public header) can be written in parallel with tests

### Implementation Strategy

1. **MVP**: Phase 1 + 2 + 3 (FlexLayout compiles, Measure/Arrange work) → core engine ready
2. **Add tests** Phase 4 (12 test cases covering all flexbox properties + edge cases)
3. **Public header + BUILD** Phase 5 → external consumers can use FlexLayout
