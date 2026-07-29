# Data / Entity Model: Architecture & Engineering Design

**Date**: 2026-07-29

## Overview

This document defines the key entities in the native_ui framework architecture. These entities represent the conceptual building blocks — not C++ types — that the architecture documents will formalize.

---

## Entity: Widget

| Attribute | Type | Description |
|-----------|------|-------------|
| `id` | string | Unique identifier within widget tree |
| `parent` | Widget (reference) | Owning parent, null for root |
| `children` | Widget[] (owned) | Ordered list of child widgets |
| `layout_props` | LayoutProps | Flexbox properties: direction, margin, padding, grow, etc. |
| `measured_size` | Size | Result of Measure phase |
| `arranged_position` | Point | Result of Arrange phase |
| `state` | WidgetState | Created → Mounted → Measured → Arranged → Ready |

**Relationships**:
- A Widget **has** children (composition)
- A Widget **has** LayoutProps
- A Widget **participates in** Layout phase (Measure → Arrange)
- A Widget **participates in** Render phase (Draw)

**Validation Rules**:
- Widget ID must be unique within the widget tree
- A widget cannot be its own ancestor (no cycles)
- Children list order determines layout order

---

## Entity: LayoutProps

| Attribute | Type | Description |
|-----------|------|-------------|
| `direction` | FlexDirection | row or column |
| `justify_content` | JustifyContent | flex-start, center, flex-end, space-between, space-around |
| `align_items` | AlignItems | stretch, flex-start, center, flex-end |
| `flex_wrap` | FlexWrap | nowrap, wrap, wrap-reverse |
| `margin` | EdgeInsets | Space around element |
| `padding` | EdgeInsets | Space inside element |
| `gap` | float | spacing between children |
| `flex_grow` | float | Proportion of remaining space to take |
| `flex_shrink` | float | Proportion to shrink when space insufficient |
| `flex_basis` | float | Default size before grow/shrink |

**Relationships**:
- LayoutProps **delegates to** Yoga for computation

---

## Entity: LayoutResult

| Attribute | Type | Description |
|-----------|------|-------------|
| `child_sizes` | Size[] | Measured sizes for each child |
| `child_positions` | Point[] | Computed positions for each child |
| `total_size` | Size | Total content size after layout |

---

## Entity: Canvas

| Attribute | Type | Description |
|-----------|------|-------------|
| `sk_canvas` | opaque handle | Underlying SkCanvas (isolated) |
| `save_count` | int | Save counter for RAII restore |
| `clip_rect` | Rect | Current clipping region |

**Relationships**:
- Canvas **wraps** SkCanvas
- Canvas **is used by** Widget::Draw()

---

## Entity: Paint

| Attribute | Type | Description |
|-----------|------|-------------|
| `color` | Color | RGBA color |
| `anti_alias` | bool | Enable/disable anti-aliasing |
| `stroke_width` | float | Stroke width (0 = fill) |
| `style` | PaintStyle | fill, stroke, fill-and-stroke |

---

## Entity: Event

| Attribute | Type | Description |
|-----------|------|-------------|
| `type` | EventType | MouseDown, MouseUp, KeyDown, KeyUp, Touch |
| `position` | Point | Event position (for mouse/touch) |
| `key_code` | int | Key code (for keyboard events) |
| `button` | MouseButton | Which button (for mouse events) |
| `target` | Widget | Widget that received the event |
| `propagation` | PropagationState | bubbling, capture, stopped |

**State Transitions**: Created → Dispatch → Bubble → Capture → Handled

---

## Entity: HitTestResult

| Attribute | Type | Description |
|-----------|------|-------------|
| `widget` | Widget | The topmost widget at the hit point |
| `position` | Point | Point in widget's local coordinate space |
| `depth` | int | Nesting depth (0 = root) |

---

## Entity: ViewModel

| Attribute | Type | Description |
|-----------|------|-------------|
| `properties` | Property[] | Observable properties with change notification |
| `bound_widgets` | Widget[] (weak ref) | Widgets currently bound to this ViewModel |
| `state` | ViewModelState | Idle → Notifying |

**Relationships**:
- ViewModel **notifies** bound Widgets on property change
- ViewModel **bridges** worker threads → main thread (properties updated on worker, notification delivered on main)

**Validation Rules**:
- Property updates must be thread-safe (lock-protected)
- Widgets must unbind before ViewModel destruction
- Rapid property changes should batch-trigger a single RequestRedraw

---

## Entity: LogSink

| Attribute | Type | Description |
|-----------|------|-------------|
| `level` | LogLevel | debug, info, warn, error |
| `message` | string | Log message text |
| `metadata` | Metadata[] | Structured key-value pairs |

**Relationships**:
- LogSink **is called by** any framework module
- LogSink **is implemented by** consumer (plug-in pattern)

**Validation Rules**:
- If no LogSink registered, Log() is a no-op
- Must be thread-safe (called from any thread)

---

## Entity: MainThread

| Attribute | Type | Description |
|-----------|------|-------------|
| `work_queue` | Task[] | Queue of render/layout/event tasks |
| `frame_deadline` | Time | Target completion time (16ms for 60fps) |

**Responsible for**:
- Event dispatch and hit testing
- Layout measure + arrange
- Skia rendering (Draw)
- ViewModel property observation → RequestRedraw
- LogSink dispatch

---

## Entity: WorkerThread

| Attribute | Type | Description |
|-----------|------|-------------|
| `pool_size` | int | Number of worker threads (configurable) |
| `task_queue` | Task[] | Business logic tasks |

**Responsible for**:
- Business logic execution
- Data processing / I/O
- ViewModel property updates (thread-safe)

**Must never**:
- Call Skia drawing APIs
- Manipulate widget tree directly
- Block the main thread

---

## Entity: Module

| Attribute | Type | Description |
|-----------|------|-------------|
| `name` | string | Module name (core, layout, render, etc.) |
| `dependencies` | Module[] | Internal module dependencies |
| `external_deps` | string[] | External third-party dependencies |
| `visibility` | string[] | Bazel visibility rules |
| `public_headers` | string[] | Exposed header files |

**State Transitions**: N/A (static design entity)

---

## State Diagram: Widget Lifecycle

```
Created → Mounted → Measured → Arranged → Ready → [Unmounted]
              ↓                                     ↑
           [dynamic update] ← insert/remove/modify →┘
```

| State | Triggers | Allowed Actions |
|-------|----------|-----------------|
| Created | Widget() constructor | SetId, AddChild |
| Mounted | Mount() called | Measure() |
| Measured | Measure() completes | Arrange() |
| Arranged | Arrange() completes | Draw() |
| Ready | Draw() completes | RequestLayout() → Measured, RequestRedraw() → Draw |
| Unmounted | Unmount() called | Destructor |
