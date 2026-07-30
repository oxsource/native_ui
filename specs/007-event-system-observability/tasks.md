---

description: "Task list for Phase 7: Event System & Observability"

---

# Tasks: Event System & Observability

**Input**: Design documents from `specs/007-event-system-observability/`

**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Test tasks are included per the spec's acceptance criteria.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

All paths are relative to `native_ui/` under the repository root.

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Create event module BUILD, public headers, and project structure

- [x] T001 Create `event` cc_library in `native_ui/src/framework/event/BUILD.bazel` with deps on `//src/framework/core`, `//src/framework/widgets`
- [x] T002 [P] Create public re-export header `native_ui/src/framework/public/include/native_ui/event.h` for EventHub, HitTester, DispatchResult
- [x] T003 [P] Create public re-export header `native_ui/src/framework/public/include/native_ui/debug_overlay.h` for DebugOverlay

---

## Phase 2: Foundational (Blocking Prerequisites) 🎯 MVP

**Purpose**: Define shared event types (MouseEvent, KeyEvent) and DispatchResult — used by ALL user stories.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T004 Create `dispatch_result.h` in `native_ui/src/framework/event/dispatch_result.h` with `DispatchStatus` enum (kHandled, kUnhandled, kRejected, kNoTarget) and `DispatchResult` struct (status + target Widget*)
- [x] T005 [P] Create `event_types.h` in `native_ui/src/framework/event/event_types.h` with `MouseEvent` (position, button, modifiers), `KeyEvent` (key_code, modifiers), `MouseButton`, `ModifierFlags` structs

**Checkpoint**: Foundation ready — event types and DispatchResult compile.

---

## Phase 3: User Story 2 - HitTester (Priority: P1)

**Goal**: HitTester performs DFS on widget tree, returning deepest widget at a given point. Respects Stack z-order (topmost tested first). Returns local coordinate.

**Independent Test**: Create Container with two overlapping children, call HitTester::Test at overlap point, verify deepest widget returned. Test outside bounds returns null.

- [x] T006 [P] [US2] Create `HitTester` header with `Test(Widget* root, Point point) -> HitTestResult` and `HitTestResult` struct in `native_ui/src/framework/event/hit_tester.h`
- [x] T007 [US2] Implement `HitTester::Test` DFS algorithm in `native_ui/src/framework/event/hit_tester.cc` — iterate Container/Stack children in reverse (topmost first), check `bounds().Contains(point)`, return deepest match with local_pos

**Checkpoint**: HitTester correctly returns deepest widget for overlapping and non-overlapping cases.

---

## Phase 4: User Story 1 - EventHub + Dispatch Protocol (Priority: P1)

**Goal**: EventHub provides `Push(MouseEvent)` with full dispatch pipeline (filter → hit test → capture → target → bubble). Widget base gains virtual `OnMouseEvent`/`OnKeyEvent` methods.

**Independent Test**: Create widget tree with Button, push MouseEvent at button position, verify kHandled and callback invoked. Push outside bounds, verify kNoTarget.

- [ ] T008 [P] [US1] Add virtual `OnMouseEvent(const MouseEvent&) -> bool` and `OnKeyEvent(const KeyEvent&) -> bool` methods to `Widget` base class in `native_ui/src/framework/widgets/widget.h` (default returns false)
- [ ] T009 [US1] Update `Button` in `native_ui/src/framework/widgets/button.h` / `button.cc` to override `OnMouseEvent` — check `bounds().Contains(event.position)`, invoke `on_click_`, return true
- [ ] T010 [P] [US1] Create `EventHub` header with `Push(MouseEvent)`, `Push(KeyEvent)`, `AddFilter`, `hit_tester()` in `native_ui/src/framework/event/event.h`
- [ ] T011 [US1] Implement `EventHub::Push(MouseEvent)` dispatch pipeline in `native_ui/src/framework/event/event.cc` — filter → hit test → capture → target → bubble; implement `Push(KeyEvent)` direct dispatch to focused widget
- [ ] T012 [US1] Implement `EventHub::Push(KeyEvent)` in `native_ui/src/framework/event/event.cc` — walk tree to find widget that handles the key

**Checkpoint**: EventHub dispatches MouseEvent through full pipeline, returns correct DispatchResult.

---

## Phase 5: User Story 3 - Event Filter Chain (Priority: P2)

**Goal**: EventHub supports `AddFilter(predicate)` to register filters that can reject events before dispatch, returning kRejected.

**Independent Test**: Register filter that rejects events at position x<10. Push MouseEvent at x=5, verify kRejected. Push at x=50, verify dispatch proceeds normally.

- [ ] T013 [P] [US3] Add `using EventFilter = function<bool(const MouseEvent&)>` and `AddFilter(EventFilter)` method to EventHub in `native_ui/src/framework/event/event.h`
- [ ] T014 [US3] Implement filter chain iteration in `EventHub::Push()` in `native_ui/src/framework/event/event.cc` — evaluate filters in registration order, return kRejected if any filter returns false

**Checkpoint**: Filters can reject events before dispatch; non-matching filters allow dispatch.

---

## Phase 6: User Story 4 - DebugOverlay (Priority: P2)

**Goal**: DebugOverlay widget draws layout borders, FPS counter, and widget tree breadcrumb. Toggleable via F12. Compiled out in NDEBUG builds.

**Independent Test**: Create DebugOverlay, toggle on, verify layout borders drawn. Set FPS, verify display. Verify NDEBUG guard excludes code in release build.

- [ ] T015 [P] [US4] Create `DebugOverlay` header in `native_ui/src/framework/widgets/debug_overlay.h` with `Toggle()`, `set_fps(int)`, `set_breadcrumb(string)`, `Draw(Canvas&)`, `OnKeyEvent` override
- [ ] T016 [US4] Implement `DebugOverlay::Draw` in `native_ui/src/framework/widgets/debug_overlay.cc` — walk widget tree drawing colored border rects per depth, render FPS text, render breadcrumb text; no-op when `!visible_`
- [ ] T017 [US4] Guard all DebugOverlay implementation with `#ifndef NDEBUG` — empty stubs in release builds
- [ ] T018 [US4] Add `{F12}` toggle handling in `DebugOverlay::OnKeyEvent` — flip `visible_` on F12 press

**Checkpoint**: DebugOverlay toggles, draws borders/FPS/breadcrumb, excluded from release builds.

---

## Phase 7: Tests & Verification

**Purpose**: Unit tests covering all event system components and DebugOverlay

- [ ] T019 [P] Write DispatchResult and event types unit tests in `native_ui/tests/event_test.cc` — enum values, struct construction
- [ ] T020 [P] Write HitTester unit tests in `native_ui/tests/event_test.cc` — single widget, overlapping children, Stack z-order, null root, no hit
- [ ] T021 [P] Write EventHub Push unit tests in `native_ui/tests/event_test.cc` — mock MouseEvent hitting Button (kHandled), outside bounds (kNoTarget), empty tree (kNoTarget)
- [ ] T022 [P] Write event filter chain unit tests in `native_ui/tests/event_test.cc` — filter rejects (kRejected), filter allows (kHandled), multiple filters
- [ ] T023 [P] Write DebugOverlay unit tests in `native_ui/tests/debug_overlay_test.cc` — toggle on/off, layout borders draw, FPS display, NDEBUG guard
- [ ] T024 Add `event_test` and `debug_overlay_test` cc_test targets in `native_ui/tests/BUILD.bazel` with deps on `//src/framework/event`, `//src/framework/widgets`, `//src/framework/render`
- [ ] T025 Verify build and tests pass: `bazel build //src/framework/event //src/framework/widgets` and `bazel test //tests:event_test //tests:debug_overlay_test`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup — BLOCKS all user stories
- **US2 HitTester (Phase 3)**: Depends on Phase 2 — standalone, no deps on other stories
- **US1 EventHub (Phase 4)**: Depends on Phase 2 + Phase 3 (HitTester used in dispatch)
- **US3 Filters (Phase 5)**: Depends on Phase 4 (AddFilter is part of EventHub)
- **US4 DebugOverlay (Phase 6)**: Depends on Phase 2 only — independent of other stories
- **Tests (Phase 7)**: Depends on all user story implementations

### User Story Dependencies

- **US2 (HitTester) P1**: No dependencies on other stories — standalone
- **US1 (EventHub) P1**: Depends on US2 (HitTester) — uses it in dispatch pipeline
- **US3 (Filters) P2**: Depends on US1 (EventHub) — filters execute inside Push()
- **US4 (DebugOverlay) P2**: No dependencies on other stories — standalone widget

### Within Each User Story

- Header before implementation
- Core implementation before edge case handling
- Story complete before moving to next priority

### Parallel Opportunities

- T001, T002, T003 (Setup) — all [P], independent files
- T004, T005 (Foundation) — sequential, but T005 is [P] (independent file)
- T006+T007 (US2 HitTester) — sequential (header → impl)
- T008+T009+T010+T011+T012 (US1 EventHub) — T008 and T010 are [P] (different files), sequential within each file
- T013+T014 (US3 Filters) — T013 is [P] (header update), T014 is impl
- T015+T016+T017+T018 (US4 DebugOverlay) — T015 and T017 are [P], T016+T018 sequential
- T019–T023 (Tests) — all [P], independent test sections
- US4 (DebugOverlay) can run IN PARALLEL with US1/US2/US3 since it's standalone

---

## Parallel Example: All P1 Stories

```bash
# US2 (HitTester) — independent:
Task: "Create hit_tester.h and hit_tester.cc"

# US1 (EventHub) — after US2:
Task: "Add OnEvent to Widget, create event.h and event.cc"

# US4 (DebugOverlay) — fully parallel with US2:
Task: "Create debug_overlay.h and debug_overlay.cc"
```

---

## Implementation Strategy

### MVP First (User Story 2 + User Story 1)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundation (DispatchResult + event types)
3. Complete Phase 3: US2 (HitTester) — standalone, independently testable
4. Complete Phase 4: US1 (EventHub + dispatch) — depends on HitTester
5. **STOP and VALIDATE**: `bazel test //tests:event_test`

### Incremental Delivery

1. Setup + Foundation → event types and DispatchResult ready
2. US2 (HitTester) → hit testing works on widget tree
3. US1 (EventHub) → full event dispatch with bubble protocol
4. US3 (Filters) → event interception capability
5. US4 (DebugOverlay) → visual debugging tool
6. Phase 7 → All tests green

### Parallel Team Strategy

1. Team completes Phase 1 + Phase 2 together
2. Once Phase 2 is done:
   - Developer A: US2 (HitTester) → US1 (EventHub)
   - Developer B: US4 (DebugOverlay) — fully parallel
3. Developer A completes US3 (Filters) after US1
4. All tests written and verified

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- HitTester is a separate file/class from EventHub — clean separation
- EventHub::Push uses HitTester internally — US1 depends on US2
- Widget::OnMouseEvent/OnKeyEvent are virtual with default false — backward compatible
- DebugOverlay is compiled out with #ifndef NDEBUG — no impact on release builds
- All tests use mock input events (no platform dependency per clarification)
