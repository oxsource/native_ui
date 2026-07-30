# Feature Specification: Event System & Observability

**Feature Branch**: `007-event-system-observability`

**Created**: 2026-07-30

**Status**: Draft

**Input**: User description: "Phase 7: Event System & Observability"

## Clarifications

### Session 2026-07-30

- Q: 测试验证方式是否需要对接具体平台事件源 → A: 不需要，通过构造 Mock 输入事件（MouseEvent/KeyEvent）直接调用 EventHub::Push 验证即可，不依赖任何平台事件循环

## User Scenarios & Testing

### User Story 1 - Developer Pushes Events Into the Framework (Priority: P1)

A developer receives platform-level events (mouse click at position x,y) and pushes them into native_ui via `EventHub`. The hub returns a `DispatchResult` indicating whether the event was handled by a widget, unhandled, or rejected.

**Why this priority**: EventHub is the single entry point for all external events. Without it, the framework cannot receive any user input. Every interactive UI depends on this.

**Independent Test**: A developer creates a simple widget tree with a Button, pushes a mouse click at the button's position, and verifies the return status is `kHandled` and the button's click callback was invoked.

**Acceptance Scenarios**:

1. **Given** a widget tree with a Button at position (10,10,80,40), **When** a `MouseEvent` at (50,30) is pushed via `EventHub`, **Then** `DispatchResult.status` is `kHandled` and the button's callback is invoked
2. **Given** a widget tree with a Button, **When** a `MouseEvent` outside any widget bounds is pushed, **Then** `DispatchResult.status` is `kNoTarget`
3. **Given** a widget tree, **When** a `KeyEvent` is pushed to a valid widget, **Then** the event reaches the widget through the dispatch pipeline

---

### User Story 2 - Hit Testing Finds the Deepest Widget at a Point (Priority: P1)

A developer uses `HitTester` to determine which widget in the tree is at a given coordinate. The hit test returns the deepest (most specific) widget and the local coordinate relative to that widget's bounds.

**Why this priority**: Hit testing is the foundation of all mouse/touch event dispatch. The hub must know which widget an event targets before it can dispatch.

**Independent Test**: A developer creates a Container with two overlapping children at different depths, calls `HitTester::Test` at the overlap point, and verifies the deepest visible widget is returned.

**Acceptance Scenarios**:

1. **Given** a Container with child A (z=0) and child B (z=1) overlapping, **When** `HitTester::Test` is called at the overlap point, **Then** child B (topmost) is returned
2. **Given** a Container with a Button at known bounds, **When** `HitTester::Test` is called outside the button, **Then** the parent Container is returned
3. **Given** an empty area with no widgets, **When** `HitTester::Test` is called, **Then** null is returned

---

### User Story 3 - Developer Uses Event Filtering (Priority: P2)

A developer registers an event filter to intercept events before dispatch — for example, to implement a keyboard shortcut or debug gesture.

**Why this priority**: Filters enable cross-cutting concerns like keyboard shortcuts, accessibility, and debugging without modifying individual widgets.

**Independent Test**: A developer registers a filter that rejects events at a specific position, pushes a MouseEvent at that position, and verifies the result is `kRejected`.

**Acceptance Scenarios**:

1. **Given** an EventHub with a registered filter, **When** a matching event is pushed, **Then** `DispatchResult.status` is `kRejected`
2. **Given** an EventHub with a filter that does NOT match the incoming event, **When** the event is pushed, **Then** dispatch proceeds normally to widgets

---

### User Story 4 - Developer Uses DebugOverlay (Priority: P2)

A developer toggles a visual overlay during development to inspect widget layout borders, FPS, and the widget tree breadcrumb under the cursor.

**Why this priority**: DebugOverlay is the primary debugging tool for widget layout and rendering issues. It accelerates development without needing external profiling tools.

**Independent Test**: A developer creates a DebugOverlay attached to a widget tree, toggles it on, and verifies the overlay renders layout borders around all widgets and displays FPS information.

**Acceptance Scenarios**:

1. **Given** a DebugOverlay attached to a widget tree, **When** toggled on, **Then** colored layout borders are drawn around each widget
2. **Given** a DebugOverlay toggled on, **When** FPS data is available, **Then** the overlay displays the current FPS count
3. **Given** a DebugOverlay, **When** the widget tree breadcrumb is computed, **Then** it shows the path from root to widget under cursor
4. **Given** a release build, **When** DebugOverlay code is compiled, **Then** it is excluded via NDEBUG guard

---

### Edge Cases

- What happens when HitTester is called on a null root?
- What happens when EventHub::Push is called with a null or invalid event?
- What happens when multiple filters return conflicting results?
- What happens when an event reaches a widget that has been unmounted?
- What happens when DebugOverlay is toggled on an empty tree?
- What happens when a widget does not override any event handler (no-op)?

## Requirements

### Functional Requirements

- **FR-001**: EventHub MUST provide a unified `Push(event)` entry point for at least `MouseEvent` — returns `DispatchResult`
- **FR-002**: EventHub MUST support a filter chain — `AddFilter` registers a predicate that can reject events before dispatch
- **FR-003**: EventHub MUST use the four-phase dispatch protocol: filter → capture → target → bubble
- **FR-004**: `DispatchResult` MUST contain a `DispatchStatus` enum and an optional pointer to the target `Widget`
- **FR-005**: `DispatchStatus` MUST include `kHandled`, `kUnhandled`, `kRejected`, and `kNoTarget`
- **FR-006**: HitTester MUST perform DFS hit testing on a widget tree, returning the deepest widget that contains the point and the local coordinate
- **FR-007**: HitTester MUST return null when no widget contains the point
- **FR-008**: HitTester MUST respect widget stacking within Stack (z-order) — topmost child is tested first
- **FR-009**: DebugOverlay MUST be a Widget that renders layout borders, FPS, and widget tree breadcrumb under the cursor
- **FR-010**: DebugOverlay MUST be toggleable via a configurable shortcut (default F12)
- **FR-011**: DebugOverlay MUST be compiled out in release builds via `#ifndef NDEBUG`
- **FR-012**: DebugOverlay MUST NOT affect the layout of its parent — it is a visual-only overlay
- **FR-013**: EventHub, HitTester, DispatchResult MUST be exported via `src/framework/public/include/native_ui/event.h`
- **FR-014**: DebugOverlay MUST be exported via `src/framework/public/include/native_ui/debug_overlay.h`
- **FR-015**: Events MUST be externally injected — the framework does not own platform event loops
- **FR-016**: Push and AddFilter MUST be main-thread only

### Key Entities

- **EventHub**: Unified entry point for external events. Manages filters and coordinates hit testing + dispatch.
- **HitTester**: DFS tree walker that finds the deepest widget at a given coordinate. Respects z-order in Stack widgets.
- **DispatchResult**: Outcome of an event dispatch — includes status (handled/unhandled/rejected/no-target) and optional target widget pointer.
- **Event Types**: `MouseEvent` (position, button, modifiers), `KeyEvent` (key_code, modifiers), `TouchEvent` (position, finger_id, pressure).
- **DebugOverlay**: Special widget for development diagnostics — renders layout borders, FPS counter, and widget tree breadcrumb. Compiled out in release.

## Success Criteria

### Measurable Outcomes

- **SC-001**: A developer can push a MouseEvent that hits a Button and verify the callback fires — test completes in under 100ms
- **SC-002**: HitTester correctly returns the deepest widget for 10+ overlapping test cases with varying z-order
- **SC-003**: A filter registered via AddFilter successfully rejects events — verified by kRejected status
- **SC-004**: DebugOverlay renders layout borders and FPS when toggled on — verified by pixel readback
- **SC-005**: DebugOverlay is excluded from release builds — verified by checking NDEBUG guard
- **SC-006**: A dispatch to an empty area returns kNoTarget without crashing
- **SC-007**: All event tests pass with 100% accuracy — no false positives in hit testing or dispatch

## Assumptions

- MVP event types are `MouseEvent` and `KeyEvent` — `TouchEvent` is deferred as it requires platform-specific gesture infrastructure
- Widgets must opt into event handling by overriding an `OnEvent` method or similar — widgets that do not override return false (unhandled)
- Button from Phase 6 is the primary consumer — its internal HitTest is superseded by the centralized HitTester for event dispatch
- DebugOverlay is an optional widget — the consumer must explicitly attach it to the tree; it is not automatically injected
- Frame clock/FPS counting is provided by the consumer — DebugOverlay reads an externally-provided FPS value
- The default toggle shortcut is F12 on desktop platforms
- Filter callbacks are synchronous and run on the main thread
- All tests use mock input events constructed programmatically — no platform event loop integration required
