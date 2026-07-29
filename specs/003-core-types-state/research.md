# Research: Core Types & Widget Foundation + State

**Date**: 2026-07-29

## Decisions

### Core Types: Header-Only Value Types

- **Decision**: Rect, Point, Size, Color, EdgeInsets are header-only (`#pragma once`, no .cc). All methods are inline or constexpr.
- **Rationale**: These types are used pervasively; header-only avoids linker overhead and enables compiler inlining. They are small enough (4 × float/int each) that ODR concerns are negligible.
- **Alternatives considered**: .h/.cc pairs (rejected: unnecessary compilation overhead for trivial types)

### Color: uint8_t RGBA, Clamp on Construction

- **Decision**: Color stores RGBA as `uint8_t[4]`. Out-of-range values are clamped on construction. Named constants via `constexpr static Color`.
- **Rationale**: uint8_t matches Skia's `SkColor` format (kRGBA_8888), minimizing conversion overhead. Clamp prevents silent data corruption.

### State + Property<T>: Mutex-Protected Update, Single-Producer Queue

- **Decision**: `State` holds a `std::mutex` per instance. `Property<T>::operator=` locks the mutex, updates the value, and pushes the Property pointer to a single-producer, single-consumer lock-free queue. The main thread drains the queue at the start of each frame.
- **Rationale**: Mutex per instance keeps contention low. SPSC lock-free queue avoids waking the main thread for every update — the queue is drained once per frame during the batch step.

### Widget Invalidation: Binary Flags

- **Decision**: `Widget` has two `bool` flags (`needs_layout_`, `needs_draw_`). `RequestLayout()` sets both (layout implies draw). `RequestRedraw()` sets only `needs_draw_`.
- **Rationale**: Simpler than a bitmask or dirty-rect tracking for MVP. Dirty-rect optimization can be added later.

### Container: FlexLayout via Forward Declaration

- **Decision**: `Container` holds a `FlexLayout layout_` member (forward-declared). The FlexLayout header is NOT included in `container.h` — only in `container.cc`.
- **Rationale**: Keeps the public header lightweight. FlexLayout depends on Yoga; isolating it prevents Yoga headers from leaking into consumers of container.h.

### Watch/Unwatch: Weak Reference List

- **Decision**: `State` stores a `std::vector<Widget*>` of watching widgets (raw pointers). On `Unwatch`, the pointer is removed. On State destruction, all pointers are iterated and notification is skipped.
- **Rationale**: Widget lifetime is managed by `unique_ptr` in the parent Container. Raw pointers are safe because widgets unregister on `OnUnmount`.
