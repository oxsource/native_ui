---

description: "Task list for Phase 6: Basic Widgets & Dynamic Tree + Data Binding Integration"

---

# Tasks: Basic Widgets & Dynamic Tree

**Input**: Design documents from `specs/006-basic-widgets-dynamic-tree/`

**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Test tasks are included per the plan's acceptance criteria (`bazel test //tests:widgets_test green`).

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

All paths are relative to `native_ui/` under the repository root.

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Update build configuration and public headers to support new widgets

- [x] T001 Update `widgets` cc_library in `native_ui/src/framework/widgets/BUILD.bazel` to add `//src/framework/render` dep
- [x] T002 [P] Update `native_ui/src/framework/public/include/native_ui/widgets.h` to re-export Text, Button, ImageWidget, ExternalImage, Stack headers
- [x] T003 Add `widgets_test` cc_test target in `native_ui/tests/BUILD.bazel` with dep on `//src/framework/widgets` and `//src/framework/render`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Complete the Container layout pipeline (Measure → Arrange → positioned Draw) and add bounds support to Widget base. All leaf widgets depend on Container being able to position children correctly.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T004 Add `SetBounds(Rect)` and `bounds()` to `Widget` base class in `native_ui/src/framework/widgets/widget.h` — stores the layout bounds assigned by parent Container for hit testing and position reference
- [x] T005 Integrate FlexLayout into Container: add `FlexLayout layout_` member and `std::vector<MeasureResult> layout_result_` to `native_ui/src/framework/widgets/container.h`; add dep on `//src/framework/layout` to `native_ui/src/framework/widgets/BUILD.bazel`
- [x] T006 Implement `Container::Measure(Size)` and `Container::Arrange(Size)` in `native_ui/src/framework/widgets/container.cc` — Measure delegates to FlexLayout, Arrange stores positions and calls `child->SetBounds()` for each child
- [x] T007 Rewrite `Container::Draw(Canvas&)` in `native_ui/src/framework/widgets/container.cc` — iterate children: `Save()`, `Translate(pos)`, `ClipRect(size)`, `child->Draw(canvas)`, `Restore()`

**Checkpoint**: Foundation ready — Container properly positions children via FlexLayout, translates/clips canvas per child, and sets child bounds for hit testing.

---

## Phase 3: User Story 1 - Text Widget with Data Binding (Priority: P1) 🎯 MVP

**Goal**: Text widget renders dynamic content, supports `Watch(Property<T>&)` for auto-redraw on state change.

**Independent Test**: Create Text with Content("Hello"), draw on Canvas, verify pixel output matches expected text. Bind Text to a State property, change property, verify redraw.

### Implementation for User Story 1

- [x] T008 [P] [US1] Create `Text` header with tagged-parameter construction (Content, FontSize, Color, Id tags) in `native_ui/src/framework/widgets/text.h`
- [x] T009 [US1] Implement `Text::Draw(Canvas&)` using `Canvas::DrawText()` and Watch/UnwatchAll for data binding in `native_ui/src/framework/widgets/text.cc`

**Checkpoint**: Text widget compiles, renders text with correct position/size/color, and auto-redraws on State change.

---

## Phase 4: User Story 2 - Button with Hit Detection (Priority: P1)

**Goal**: Button widget draws a clickable label area, provides `HitTest(Point)` and `OnClick` callback, supports data binding on its label.

**Independent Test**: Create Button with Label("OK") and OnClick callback. HitTest(inside) returns true, HitTest(outside) returns false. Callback invoked on hit. State-bound label updates on property change.

### Implementation for User Story 2

- [x] T010 [P] [US2] Create `Button` header with tagged-parameter construction (Label, OnClick, Id tags) and `HitTest(Point)` method in `native_ui/src/framework/widgets/button.h`
- [x] T011 [US2] Implement `Button::Draw(Canvas&)` with background rect + centered text, `HitTest()` using `bounds()` from Widget base, `OnClick` dispatch, and data binding in `native_ui/src/framework/widgets/button.cc`

**Checkpoint**: Button renders label, detects hits correctly, invokes callback, and updates on State change.

---

## Phase 5: User Story 4 - ExternalImage with Hardware Buffers (Priority: P1)

**Goal**: ExternalImage widget renders platform hardware buffers (IOSurface/AHardwareBuffer/DMA-BUF) via zero-copy GPU import, with `SetBuffer()` and `Watch(Property<HardwareBuffer>&)` for per-frame updates.

**Independent Test**: Create ExternalImage with a valid HardwareBuffer, draw on Canvas, verify pixel output. Call `SetBuffer()` with new buffer, verify redraw. Invalid buffer produces no render and no crash.

### Implementation for User Story 4

- [ ] T012 [P] [US4] Create `ExternalImage` header with tagged-parameter construction (HardwareBuffer, Id tags), `SetBuffer(HardwareBuffer)`, and `Watch(Property<HardwareBuffer>&)` support in `native_ui/src/framework/widgets/external_image.h`
- [ ] T013 [US4] Implement `ExternalImage::Draw(Canvas&)` using `Canvas::DrawImage(Image::FromBuffer(...))`, `SetBuffer()` lifecycle, and data binding in `native_ui/src/framework/widgets/external_image.cc`

**Checkpoint**: ExternalImage renders hardware buffers, updates on `SetBuffer()` or State property change, handles invalid buffers gracefully.

---

## Phase 6: User Story 3 - File-Based Image Widget (Priority: P2)

**Goal**: Image widget decodes and renders PNG/JPEG from file path, handles missing/invalid files gracefully.

**Independent Test**: Create Image widget with valid file path, draw on Canvas, verify pixel output. Create with nonexistent path — verify no render and no crash.

### Implementation for User Story 3

- [ ] T014 [P] [US3] Create `ImageWidget` header with tagged-parameter construction (ImagePath, Id tags) in `native_ui/src/framework/widgets/image.h`
- [ ] T015 [US3] Implement `ImageWidget::Draw(Canvas&)` using `Canvas::DrawImage(Image::FromFile(...))` and error handling for missing/corrupted files in `native_ui/src/framework/widgets/image.cc`

**Checkpoint**: Image widget loads and renders PNG/JPEG files, handles missing files without crash.

---

## Phase 7: User Story 5 - Stack Widget (Z-Order) (Priority: P2)

**Goal**: Stack widget layers children in z-order (index 0 = bottom, N = top), supports dynamic AddChild/RemoveChild with RequestLayout.

**Independent Test**: Create Stack with 2+ overlapping children, draw on Canvas, verify top child renders over bottom. AddChild triggers re-layout. RemoveChild removes from render list.

### Implementation for User Story 5

- [ ] T016 [P] [US5] Create `Stack` header with tagged-parameter construction (Children, Id tags), `AddChild`, `RemoveChild`, `ClearChildren` in `native_ui/src/framework/widgets/stack.h`
- [ ] T017 [US5] Implement `Stack::Draw(Canvas&)` with z-order iteration (0=bottom, N=top), save/restore per child, `ChildAt()`, `ChildCount()`, `AddChild`/`RemoveChild` with `RequestLayout()` in `native_ui/src/framework/widgets/stack.cc`

**Checkpoint**: Stack renders children in correct z-order, AddChild/RemoveChild triggers layout.

---

## Phase 8: Tests & Verification

**Purpose**: Unit tests covering all widgets, edge cases, and build verification

- [ ] T018 [P] Write Text widget tests in `native_ui/tests/widgets_test.cc` — rendering, data binding, edge cases (empty content, zero font size)
- [ ] T019 [P] Write Button widget tests in `native_ui/tests/widgets_test.cc` — hit detection (inside/outside), callback invocation, data binding, empty label
- [ ] T020 [P] Write Image widget tests in `native_ui/tests/widgets_test.cc` — file loading, DrawImage output, missing/corrupted file handling
- [ ] T021 [P] Write ExternalImage widget tests in `native_ui/tests/widgets_test.cc` — buffer rendering, SetBuffer redraw, invalid buffer handling, data binding
- [ ] T022 [P] Write Stack widget tests in `native_ui/tests/widgets_test.cc` — z-order rendering, AddChild/RemoveChild triggers RequestLayout, empty stack
- [ ] T023 Verify build and tests pass: `bazel build //src/framework/widgets` and `bazel test //tests:widgets_test`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup — Integrates FlexLayout into Container, adds bounds to Widget base. **BLOCKS all user stories** because Container must position children correctly
- **User Stories (Phases 3–7)**: Depends on Foundational (Phase 2) completion
  - All 5 user stories are INDEPENDENT of each other — can be implemented in any order or in parallel
  - Prioritization: US1 (Text), US2 (Button), US4 (ExternalImage) are P1; US3 (Image), US5 (Stack) are P2
- **Tests (Phase 8)**: Depends on all user story implementations

### User Story Dependencies

- **US1 (Text) P1**: No dependencies on other stories — standalone widget
- **US2 (Button) P1**: No dependencies on other stories — standalone widget
- **US4 (ExternalImage) P1**: No dependencies on other stories — standalone widget
- **US3 (Image) P2**: No dependencies on other stories — standalone widget
- **US5 (Stack) P2**: No dependencies on other stories — standalone container widget

### Within Each User Story

- Header before implementation
- Core implementation before edge case handling
- Story complete before moving to next priority

### Parallel Opportunities

- T001, T002, T003 (Setup) — all [P], independent files
- T004–T007 (Foundational) — sequential (bounds → FlexLayout → Measure/Arrange → Draw rewrite)
- T008+T009 (US1 Text) — sequential (header → impl), independent of other stories
- T010+T011 (US2 Button) — sequential, parallel with US1
- T012+T013 (US4 ExternalImage) — sequential, parallel with US1, US2
- T014+T015 (US3 Image) — sequential, parallel with all P1 stories
- T016+T017 (US5 Stack) — sequential, parallel with all image stories
- T018–T023 (Tests) — all [P], independent test sections in one file
- Maximum parallelism: All 5 user stories can run concurrently by different developers AFTER Phase 2 completes

---

## Parallel Example: User Story 1

```bash
# Launch header and impl for Text:
Task: "Create text.h in native_ui/src/framework/widgets/text.h"
Task: "Create text.cc in native_ui/src/framework/widgets/text.cc"
```

## Parallel Example: All P1 Stories

```bash
# US1 (Text):
Task: "Create text.h and text.cc"

# US2 (Button):
Task: "Create button.h and button.cc"

# US4 (ExternalImage):
Task: "Create external_image.h and external_image.cc"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 3: User Story 1 (Text widget)
3. **STOP and VALIDATE**: `bazel build //src/framework/widgets`
4. Verify Text widget renders and data binding works

### Incremental Delivery

1. Setup complete → build system ready
2. US1 (Text) → MVP: dynamic text rendering with data binding
3. US2 (Button) → Input: clickable widgets with hit detection
4. US4 (ExternalImage) → Media: hardware buffer rendering
5. US3 (Image) → Static media: file-based image display
6. US5 (Stack) → Composition: z-order layered layouts
7. Phase 8 → All tests green: `bazel test //tests:widgets_test`

### Parallel Team Strategy

With multiple developers:

1. One developer completes Phase 1 (Setup)
2. Once Phase 1 is done:
   - Developer A: US1 (Text) + US3 (Image)
   - Developer B: US2 (Button) + US5 (Stack)
   - Developer C: US4 (ExternalImage)
3. All stories are independent — no merge conflicts on files
4. Phase 8 (Tests) can be distributed per story

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story is independently completable and testable
- Widget base (Watch/UnwatchAll/RequestLayout/RequestRedraw) is already implemented from Phase 3
- Container has structural methods (AddChild/RemoveChild) from Phase 3 — but needs FlexLayout integration, Measure/Arrange pipeline, and positioned Draw from Phase 2 foundational tasks
- Widget::SetBounds(Rect)/bounds() is added in Phase 2 — needed by Button::HitTest and future event system
- Container::Draw must translate canvas to child position, clip to child bounds, and save/restore — per the widget-contract.md
- Stack is a non-Yoga container — children draw at (0,0) offset relative to Stack bounds, z-order by vector index
- Each user story is independently completable and testable AFTER Phase 2 foundation is done
- All widgets use `Draw(Canvas&)` as the sole render method — no Skia headers in widgets module
- `ImageWidget` class name avoids collision with `render::Image`; internal filename is `image.h`
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
