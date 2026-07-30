# Data / Entity Model: Event System & Observability

**Date**: 2026-07-30

## Entity: MouseEvent

| Field | Type | Description |
|-------|------|-------------|
| `position` | `Point` | Click position in root widget coordinate space |
| `button` | `MouseButton` | Left / right / middle |
| `modifiers` | `ModifierFlags` | Shift, Ctrl, Alt, Meta bitmask |

## Entity: KeyEvent

| Field | Type | Description |
|-------|------|-------------|
| `key_code` | `int` | Platform-independent key code |
| `modifiers` | `ModifierFlags` | Shift, Ctrl, Alt, Meta bitmask |

## Entity: DispatchResult

| Field | Type | Description |
|-------|------|-------------|
| `status` | `DispatchStatus` | kHandled / kUnhandled / kRejected / kNoTarget |
| `target` | `Widget*` | Deepest widget that received the event (nullable) |

## Entity: EventHub

| Field | Type | Description |
|-------|------|-------------|
| `filters_` | `vector<EventFilter>` | Registered filter callbacks (in order) |
| `hit_tester_` | `HitTester` | DFS hit testing engine |

**Methods**: `Push(MouseEvent)`, `Push(KeyEvent)`, `AddFilter(EventFilter)`, `hit_tester()`

## Entity: HitTester

| Field | Type | Description |
|-------|------|-------------|
| (stateless) | — | Pure function — no member state |

**Method**: `Test(Widget* root, Point point) -> HitTestResult`

## Entity: HitTestResult

| Field | Type | Description |
|-------|------|-------------|
| `widget` | `Widget*` | Deepest widget at point, or null |
| `local_pos` | `Point` | Point in widget's local coordinate space |

## Entity: DebugOverlay (Widget Subclass)

| Field | Type | Description |
|-------|------|-------------|
| `visible_` | `bool` | Overlay toggle state (default: false) |
| `fps_` | `int` | Current FPS value (set externally) |
| `breadcrumb_` | `string` | Widget tree path under cursor |

## Relationships

```
EventHub
  ├── hit_tester_: HitTester  (composition)
  ├── filters_: EventFilter[] (filter chain)
  │
  ├── Push(MouseEvent)
  │   ├── filters evaluate → reject → kRejected
  │   ├── hit_tester_.Test() → no target → kNoTarget
  │   └── dispatch (capture → target → bubble) → kHandled/kUnhandled
  │
  └── Push(KeyEvent)
      └── direct dispatch to focused widget → bubble → result

Widget (base)
  ├── ... existing ...
  └── DebugOverlay (subclass)
      ├── Draw() with NDEBUG guard
      ├── Toggle() shortcut
      └── Layout borders, FPS, breadcrumb rendering
```

## Validation Rules

| Rule | Applies To | Description |
|------|-----------|-------------|
| Null root returns null result | HitTester | `Test(nullptr, point)` → widget = null |
| Invalid position returns no target | HitTester | Point outside all widget bounds → null |
| Filter rejection short-circuits | EventHub | First rejecting filter → kRejected, no dispatch |
| Unmounted widget skipped | HitTester, EventHub | Widgets with no parent/noroot are excluded |
| Empty tree returns no target | EventHub | `Push` on tree with no widgets → kNoTarget |
| NDEBUG excludes DebugOverlay | Build | DebugOverlay code compiled only in debug builds |
