# Research: Event System & Observability

**Date**: 2026-07-30

## Decisions

### EventHub: Unified Push Entry Point

- **Decision**: `EventHub` provides a single `Push(const MouseEvent&)` / `Push(const KeyEvent&)` entry point. Overloaded by event type, no type erasure or base class hierarchy.
- **Rationale**: Minimal number of event types (Mouse, Key) for MVP. Overloaded Push is simpler than a polymorphic event base class.
- **Alternatives considered**: Polymorphic Event base class with virtual dispatch (rejected: over-engineered for 2 types); Variant-based dispatch (rejected: less readable)

### Dispatch Protocol: Capture → Target → Bubble

- **Decision**: Four-phase protocol: filter (reject/allow) → capture (root → leaf) → target (deepest widget) → bubble (leaf → root). Widget handlers return `bool` — true stops propagation (kHandled).
- **Rationale**: Matches standard DOM event model (capture + bubble), familiar to UI developers. Button's OnClick maps naturally to event handler returning true.
- **Alternatives considered**: Direct dispatch to target only (rejected: no bubble, blocks parent interception); Observer pattern (rejected: no tree dispatch)

### Hit Testing: DFS with Z-Order Awareness

- **Decision**: `HitTester::Test(root, point)` performs DFS. For each Container, reverse-iterate children (topmost first: Stack z-order, last-added-yoga). Returns deepest widget whose `bounds().Contains(point)`.
- **Rationale**: DFS naturally handles tree nesting. Reverse iteration ensures Stack z-order: children_[N] (top) tested before children_[0] (bottom).
- **Alternatives considered**: BFS (rejected: deeper widgets preferred); Coordinate-mapped spatial hash (rejected: over-engineered for MVP tree sizes)

### Event Filter Chain: Simple Predicate Vector

- **Decision**: EventHub holds a `vector<function<bool(const MouseEvent&)>>`. Filters are evaluated in registration order. Any filter returning false → rejects event → kRejected. No filter priority or naming.
- **Rationale**: MVP doesn't need named filters or priority ordering. Simple vector push_back is sufficient for filter chain.
- **Alternatives considered**: Priority-sorted filter queue (rejected: YAGNI); Named filter map (rejected: no named remove for MVP)

### DebugOverlay: Special Widget Inside Widgets Module

- **Decision**: `DebugOverlay` is a Widget subclass that draws layout borders, FPS text, and breadcrumb. It lives in `widgets/` module (not a separate overlay system). Compiled out with `#ifndef NDEBUG`.
- **Rationale**: As a Widget, DebugOverlay benefits from the existing Draw pipeline, layout system, and Canvas rendering. No need for special rendering hooks.
- **Alternatives considered**: Separate overlay rendering pass (rejected: duplicates Canvas pipeline); External compositor layer (rejected: over-engineered)

### Testing: Mock Events, No Platform Dependency

- **Decision**: All tests construct `MouseEvent` and `KeyEvent` values directly in test code and push them via `EventHub::Push()`. No platform event loop, window system, or input capture is required for testing.
- **Rationale**: EventHub is designed as an externally-injected system — mocking the input is natural and follows the architectural model. Platform integration tests are deferred.
- **Alternatives considered**: Platform event recording/replay (rejected: adds platform dependency); UI automation framework (rejected: heavy, deferred)
