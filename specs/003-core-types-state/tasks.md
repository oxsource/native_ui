---

description: "Task list for Core Types & Widget Foundation + State"

---

# Tasks: Core Types & Widget Foundation + State

**Input**: Design documents from `specs/003-core-types-state/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Organization**: Tasks are grouped by user story to enable independent implementation and testing.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

All paths are relative to the Bazel workspace root (`native_ui/`).

---

## Phase 1: Setup & BUILD Infrastructure

**Purpose**: Initialize module BUILD files and directory structure for Phase 3

- [x] T001 Update `src/framework/core/BUILD.bazel` as header-only `cc_library` with `hdrs = glob(["*.h"])` and `visibility = ["//src/framework:__subpackages__", "//tests:__subpackages__"]`
- [x] T002 Update `src/framework/viewmodel/BUILD.bazel` with `deps = ["//src/framework/core"]` and same visibility
- [x] T003 Create `src/framework/widgets/BUILD.bazel` as `cc_library` with deps on `//src/framework/core`, `//src/framework/viewmodel`, `@yoga//:yoga`
- [x] T004 Update `src/framework/public/BUILD.bazel` to add `//src/framework/core`, `//src/framework/viewmodel` to deps

**Checkpoint**: `bazel build //src/framework/core //src/framework/viewmodel //src/framework/widgets //src/framework/public:native_ui` succeeds

---

## Phase 2: Core Types — Rect & Point (User Story 1) 🎯 MVP

**Purpose**: Header-only geometry primitives

**Independent Test**: `bazel test //tests:core_test` passes with Rect and Point tests

- [x] T005 [P] [US1] Create `src/framework/core/rect.h` with `Rect` struct — fields `x`, `y`, `width`, `height`; methods `Contains(Point)`, `Intersect(Rect)`, `Union(Rect)`, `Inset(EdgeInsets)`, `Offset(Point)`
- [x] T006 [P] [US1] Create `src/framework/core/point.h` with `Point` struct — fields `x`, `y`; methods `operator+`, `operator-`, `DistanceTo`
- [x] T007 [US1] Create `tests/core_rect_test.cc` with unit tests: Rect creation, Contains (inside/outside/boundary), Intersect (overlapping/non-overlapping), Union, Inset, Offset, zero/negative dimensions
- [x] T008 [US1] Create `tests/core_point_test.cc` with unit tests: Point creation, arithmetic operators, DistanceTo

**Checkpoint**: `bazel test //tests:core_rect_test //tests:core_point_test` passes

---

## Phase 3: Core Types — Size & Color & EdgeInsets (User Story 1)

**Purpose**: Remaining core type primitives

**Independent Test**: `bazel test //tests:core_test` passes for all types

- [x] T009 [P] [US1] Create `src/framework/core/size.h` with `Size` struct — fields `width`, `height`; methods `IsEmpty()`, `operator==`
- [x] T010 [P] [US1] Create `src/framework/core/color.h` with `Color` struct — `uint8_t` RGBA channels, clamp-on-construction, constexpr namespace-level named constants (`kRed`, `kGreen`, `kBlue`, `kWhite`, `kBlack`, `kTransparent`)
- [x] T011 [P] [US1] Create `src/framework/core/edge_insets.h` with `EdgeInsets` struct — fields `top`, `left`, `bottom`, `right`; factories `All(v)`, `Symmetric(h, v)`, `Only(t, r, b, l)`
- [x] T012 [US1] Create `tests/core_type_test.cc` with unit tests: Size isEmpty/equality, Color channel access/clamping/named constants, EdgeInsets construction and application to Rect

**Checkpoint**: `bazel test //tests:core_rect_test //tests:core_point_test //tests:core_type_test` all pass

---

## Phase 4: State — Property\<T\> Base (User Story 2)

**Purpose**: Typed observable property template with extension hooks

**Independent Test**: `bazel test //tests:state_test` passes with Property tests

- [x] T013 [P] [US2] Create `src/framework/viewmodel/property_base.h` with `PropertyBase` abstract class — virtual `Signal()`, `key()` identity method
- [x] T014 [P] [US2] Create `src/framework/viewmodel/property.h` with `Property<T> : PropertyBase` template — constructor taking `State*`, `operator=` (triggers `before_set_` → value update → `Signal()` → `after_set_`), `value()`, `operator const T&()`, private `value_`, `before_set_` hook, `after_set_` hook
- [x] T015 [US2] Create `tests/property_test.cc` with unit tests: Property construction, operator= triggers Signal, value readback, multiple assignments in sequence

**Checkpoint**: `bazel test //tests:property_test` passes

---

## Phase 5: State — State Base + Thread Safety (User Story 2)

**Purpose**: State class with mutex-protected watchers and batch notification

**Independent Test**: `bazel test //tests:state_test` passes

- [x] T016 [US2] Create `src/framework/viewmodel/state.h` with `State` class — `AddWatcher(key, callback, PropertyBase*)`, `RemoveWatcher(key)`, `EnqueueDirty`, `Flush`, mutex-protected watchers and dirty queue
- [x] T017 [US2] Create `src/framework/viewmodel/state.cc` with `State` implementation — `AddWatcher`/`RemoveWatcher`, `Flush` deduplicates and calls `NotifyWatchers`, each watcher invokes stored callback
- [x] T018 [US2] Implement thread-safe batch coalescing in `state.cc` — `EnqueueDirty` locks mutex, pushes Property*, `Flush` swaps queue, deduplicates via sort+unique, single notification pass
- [x] T019 [US2] Create `tests/state_test.cc` with unit tests: Watch triggers redraw, batch coalescing, multiple properties, no crash after widget destruction (4 tests all pass)

**Checkpoint**: `bazel test //tests:state_test` passes

---

## Phase 6: Widget Base (User Story 3)

**Purpose**: Widget abstract base class with ID, tree navigation, and invalidation

**Independent Test**: `bazel test //tests:widget_test` passes with Widget tests

- [x] T020 [P] [US3] Create `src/framework/widgets/widget.h` with `Widget` base class — `SetId`, `GetId`, `FindById`, `ChildAt`, `ChildCount`, `IndexOf`, `Watch(Property<T>&)`, `UnwatchAll`, `RequestLayout`, `RequestRedraw`, `OnMount`, `OnUnmount`, `Draw(Canvas&)`, private `id_`, `needs_layout_`, `needs_draw_`
- [x] T021 [P] [US3] Create `src/framework/widgets/widget.cc` with implementation — `FindById` DFS traversal, `RequestLayout` sets both `needs_layout_` and `needs_draw_`, `RequestRedraw` sets only `needs_draw_`, `Watch` calls `State::AddWatcher`, `UnwatchAll` calls `State::RemoveWatcher` for all watched properties
- [x] T022 [US3] Create `tests/widget_test.cc` with unit tests: `SetId`/`GetId` roundtrip, `FindById` finds correct child, `FindById` returns null for nonexistent ID, `ChildAt`/`ChildCount` for leaf widget, `RequestLayout` sets both dirty flags

**Checkpoint**: `bazel test //tests:widget_test` passes

---

## Phase 7: Container Widget (User Story 3)

**Purpose**: Composite widget with tagged-parameter constructor, child management, and FlexLayout integration

**Independent Test**: `bazel test //tests:widget_test` passes with Container tests

- [x] T023 [P] [US3] Create `src/framework/widgets/container.h` with `Container : Widget` class — tagged-parameter `template<typename... Args> explicit Container(Args&&...)` with C++17 fold expression, `Children` struct, `Direction`, `Padding`, `Gap`, `Margin`, `Id` tag types, `AddChild`, `RemoveChild`, `ClearChildren`, `ChildAt`/`ChildCount` overrides, `YGNodeRef root_` for Yoga
- [x] T024 [P] [US3] Implement `ProcessArg` dispatch methods in `container.cc` — `ProcessArg(Direction)` calls `YGNodeStyleSetFlexDirection`, `ProcessArg(Padding)` calls `YGNodeStyleSetPadding`, `ProcessArg(Gap)` calls `YGNodeStyleSetGap`, `ProcessArg(Children)` calls `AddChild` for each child
- [x] T025 [US3] Implement `AddChild` in `container.cc` — creates `YGNodeRef`, calls `YGNodeInsertChild`, stores in `children_` vector, calls `RequestLayout()`
- [x] T026 [US3] Implement `RemoveChild` and `ClearChildren` in `container.cc` — `RemoveChild` calls `YGNodeRemoveChild` + `YGNodeFree`, `ClearChildren` removes all + frees all Yoga nodes, both call `RequestLayout()`
- [x] T027 [US3] Create `tests/container_test.cc` with 8 unit tests: empty container, AddChild, AddChild triggers layout, RemoveChild, ClearChildren, FindById in tree, IndexOf, tagged construction — all pass

**Checkpoint**: `bazel test //tests:widget_test //tests:container_test` passes

---

## Phase 8: Integration Test — Container → FlexLayout (User Story 3)

**Purpose**: Cross-module verification of Container + FlexLayout + core types

**Independent Test**: `bazel test //tests:integration:container_layout_test` passes

- [ ] T028 Create `tests/integration/container_layout_test.cc` with integration test: create Container with Direction(kRow), Gap(8), Padding(12), two child Widgets; call RequestLayout; verify Measure returns non-zero child sizes; verify Arrange updates child positions
- [ ] T029 Run full validation: `bazel build //...` + `bazel test //...` — verify all targets compile and all tests pass

**Checkpoint**: `bazel test //...` passes (all 7+ test files green)

---

## Phase 9: Public Headers

**Purpose**: Expose Phase 3 types through the public API umbrella

- [ ] T030 Create `src/framework/public/include/native_ui/core.h` — re-export `Rect`, `Point`, `Size`, `Color`, `EdgeInsets`
- [ ] T031 Create `src/framework/public/include/native_ui/viewmodel.h` — re-export `State`, `Property<T>`
- [ ] T032 Update `src/framework/public/include/native_ui/widgets.h` — re-export `Widget`, `Container`

**Checkpoint**: External project can `#include "native_ui/core.h"` and use all types

---

## Dependencies & Execution Order

```
Phase 1 (Setup: T001-T004)
  │
  ├── Phase 2 (Rect & Point: T005-T008)
  │     │
  │     └── Phase 3 (Size/Color/Insets: T009-T012)
  │
  ├── Phase 4 (Property<T>: T013-T015)
  │     │
  │     └── Phase 5 (State: T016-T019)
  │
  └── Phase 6 (Widget: T020-T022)
        │
        └── Phase 7 (Container: T023-T027)
              │
              └── Phase 8 (Integration: T028-T029)

Phase 9 (Public headers: T030-T032) — depends on all above
```

### Parallel Opportunities

- T005-T006 (rect.h, point.h) ✗ sequential (both core headers, no conflicts)
- T009-T011 (size.h, color.h, edge_insets.h) **[P]** — completely independent headers
- T013-T014 (property_base.h, property.h) **[P]** — independent files
- T020-T021 (widget.h, widget.cc) **[P]** — header and impl can be written concurrently
- T023-T024 (container.h, ProcessArg methods) **[P]** — independent concerns

### Implementation Strategy

1. **MVP**: Phase 1 + Phase 2 (core Rect + Point + tests) → basic geometry usable
2. **Add Phase 3** → all core types complete
3. **Add Phase 4 + 5** → State data binding infrastructure
4. **Add Phase 6 + 7** → Widget tree with Container
5. **Add Phase 8** → integration verified
6. **Add Phase 9** → public API ready
