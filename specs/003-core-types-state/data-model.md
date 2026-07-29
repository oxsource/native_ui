# Data / Entity Model: Core Types & Widget Foundation + State

**Date**: 2026-07-29

## Entity: Rect

| Field | Type | Description |
|-------|------|-------------|
| `x` | float | Left edge |
| `y` | float | Top edge |
| `width` | float | Width (may be negative) |
| `height` | float | Height (may be negative) |

**Methods**: `Contains(Point)`, `Intersect(Rect)`, `Union(Rect)`, `Inset(EdgeInsets)`, `Offset(Point)`

## Entity: Point

| Field | Type | Description |
|-------|------|-------------|
| `x` | float | Horizontal coordinate |
| `y` | float | Vertical coordinate |

**Methods**: `operator+`, `operator-`, `DistanceTo(Point)`

## Entity: Size

| Field | Type | Description |
|-------|------|-------------|
| `width` | float | Horizontal extent |
| `height` | float | Vertical extent |

**Methods**: `IsEmpty()`, `operator==`

## Entity: Color

| Field | Type | Description |
|-------|------|-------------|
| `r` | uint8_t | Red channel (0–255) |
| `g` | uint8_t | Green channel (0–255) |
| `b` | uint8_t | Blue channel (0–255) |
| `a` | uint8_t | Alpha channel (0–255) |

**Named constants**: `kRed`, `kGreen`, `kBlue`, `kWhite`, `kBlack`, `kTransparent`

## Entity: EdgeInsets

| Field | Type | Description |
|-------|------|-------------|
| `top` | float | Top inset |
| `left` | float | Left inset |
| `bottom` | float | Bottom inset |
| `right` | float | Right inset |

**Factories**: `EdgeInsets::All(v)`, `EdgeInsets::Symmetric(h, v)`, `EdgeInsets::Only(t,r,b,l)`

## Entity: Property<T>

| Member | Description |
|--------|-------------|
| `value_` | Typed value storage (inline) |
| `before_set_` | Pre-assignment hook (`std::function<void(const T&)>`) |
| `after_set_` | Post-assignment hook (`std::function<void(const T&)>`) |

**Behavior**: `operator=` invokes `before_set_` → update `value_` → `Signal()` → `after_set_`

## Entity: State

| Member | Description |
|--------|-------------|
| `watchers_` | `std::vector<Widget*>` of watching widgets |
| `mutex_` | `std::mutex` for thread-safe Property assignment |

**State transitions**: `Idle → Notifying → Coalescing (frame batch) → Idle`

## Entity: Widget

| Member | Description |
|--------|-------------|
| `id_` | `std::string` — unique identifier |
| `needs_layout_` | `bool` — true if layout must be recalculated |
| `needs_draw_` | `bool` — true if visual must be redrawn |

**Methods**: `SetId`, `GetId`, `RequestLayout`, `RequestRedraw`, `FindById` (DFS)

## Entity: Container

| Member | Description |
|--------|-------------|
| `children_` | `std::vector<std::unique_ptr<Widget>>` — owned children |
| `layout_` | `FlexLayout` — Yoga-based layout engine (forward-declared) |

**Methods**: `AddChild`, `RemoveChild`, `ClearChildren`, `ChildAt`, `ChildCount`
