# Feature Specification: Core Types & Widget Foundation + State

**Feature Branch**: `003-core-types-state`

**Created**: 2026-07-29

**Status**: Draft

**Input**: User description: "Core Types & Widget Foundation + State"

## User Scenarios & Testing

### User Story 1 - Developer Uses Core Geometry Types (Priority: P1)

A developer writes layout logic using `Rect`, `Point`, `Size`, `Color`, and `EdgeInsets` as the foundational building blocks for all UI calculations.

**Why this priority**: These types are dependencies of every other module — layout, render, widgets, and event all depend on core types. Without them, no downstream work can proceed.

**Independent Test**: A developer can create, manipulate, and query core geometry objects (rect containment, point arithmetic, size comparison, color blending, edge inset computation) and verify results with unit tests.

**Acceptance Scenarios**:

1. **Given** a Rect and Point, **When** checking containment, **Then** the result is correct for inside/outside/boundary cases
2. **Given** two Rects, **When** computing intersection, **Then** the result is correct for overlapping and non-overlapping cases
3. **Given** a Color, **When** reading RGBA channels, **Then** values match the input specification
4. **Given** EdgeInsets, **When** applying to a Rect, **Then** the inset rect is correctly computed

---

### User Story 2 - Developer Binds State to Widget (Priority: P1)

A developer creates a `State` object with typed properties, binds it to a `Widget`, and sees the widget automatically redraw when the state changes.

**Why this priority**: Data binding is the core mechanism that connects application data to the UI. Getting this right early establishes the reactivity model for all downstream widgets.

**Independent Test**: A developer can create a `State` with `Property<int>` and `Property<string>` members, assign values via `operator=`, and verify that watching widgets receive notification and trigger `RequestRedraw`.

**Acceptance Scenarios**:

1. **Given** a State with a `Property<int>`, **When** assigning a new value via `operator=`, **Then** watching widgets are notified
2. **Given** a State with multiple properties, **When** multiple properties change in the same frame, **Then** batch coalescing produces a single notification
3. **Given** a State, **When** a value is assigned from a worker thread, **Then** the notification is delivered on the main thread
4. **Given** a Widget watching a State, **When** the Widget is unmounted, **Then** it automatically unwatches (no dangling reference)

---

### User Story 3 - Developer Builds Widget Tree with Container (Priority: P1)

A developer constructs a declarative UI using `Container` with tagged parameters, adds child widgets, and verifies layout invalidation works correctly.

**Why this priority**: The Container is the fundamental building block for all composite UIs. The tagged-parameter pattern defines the API style for all future widgets.

**Independent Test**: A developer can create a `Container` with nested children, add/remove children dynamically, trigger `RequestLayout`, and verify widget tree navigation via `FindById`.

**Acceptance Scenarios**:

1. **Given** a Container constructed with `Direction(kRow), Padding(16), Children{...}`, **When** compiled, **Then** it links correctly
2. **Given** a Container with children, **When** `FindById("child")` is called, **Then** the correct child Widget pointer is returned
3. **Given** a Container, **When** `AddChild` or `RemoveChild` is called, **Then** `RequestLayout` is triggered on the subtree
4. **Given** a Container, **When** `ClearChildren` is called, **Then** all children are destroyed and `RequestLayout` is triggered

---

### Edge Cases

- What happens when a Rect has zero or negative width/height?
- What happens when a Color component is out of range (0–255)?
- What happens when a State is destroyed while widgets are still watching it?
- What happens when Property assignment is called simultaneously from multiple threads?
- What happens when a Container with no children is used as a root widget?
- What happens when `FindById` is called with an ID that does not exist?

## Requirements

### Functional Requirements

- **FR-001**: Core geometry types (Rect, Point, Size, Color, EdgeInsets) must support construction, arithmetic, containment testing, and comparison
- **FR-002**: State must hold typed `Property<T>` members that trigger notification on `operator=` assignment
- **FR-003**: Property notification must be automatically batched within a single frame (React-style coalescing)
- **FR-004**: `Property<T>::operator=` must be thread-safe — notifiable from worker threads, delivery on main thread
- **FR-005**: State must support `Watch(Property<T>&)` for subscribing widgets and `Unwatch(Widget*)` for cleanup
- **FR-006**: Property must support extension hooks (`OnBeforeSet`, `OnAfterSet`) for value interception
- **FR-007**: Widget must provide `SetId`, `GetId`, `FindById` (DFS), `ChildAt`, `ChildCount`, `IndexOf`
- **FR-008**: Widget must support `RequestLayout` and `RequestRedraw` invalidation flags
- **FR-009**: Container must implement tagged-parameter constructor, `AddChild`, `RemoveChild`, `ClearChildren`
- **FR-010**: Container must propagate `RequestLayout` on all child mutations
- **FR-011**: Public headers must re-export core types, Widget, Container, and State

### Key Entities

- **Rect**: A rectangle defined by origin (x, y) and size (width, height), supporting containment, intersection, and union operations
- **Point**: A 2D coordinate with x and y values
- **Size**: A 2D extent with width and height
- **Color**: An RGBA color value with named constants (kRed, kBlue, etc.) and channel access
- **EdgeInsets**: Symmetric or per-side insets (top, left, bottom, right) for margin/padding
- **State**: A data holder with typed `Property<T>` members that notify watching widgets on change
- **Property\<T\>**: A typed observable value with `operator=` trigger, `Signal()`, and optional `OnBeforeSet`/`OnAfterSet` hooks
- **Widget**: The abstract base class with ID, tree navigation, layout invalidation, and `Draw(Canvas&)` pure virtual
- **Container**: A composite Widget that holds children, applies FlexLayout, and provides tree manipulation
- **Watch/Unwatch**: The subscription lifecycle between a Widget and a State's Property

## Success Criteria

### Measurable Outcomes

- **SC-001**: A developer can construct and test all 5 core types (Rect, Point, Size, Color, EdgeInsets) in under 15 minutes by following the core type spec
- **SC-002**: A developer can create a State, bind it to a Widget, and verify automatic redraw in under 10 minutes
- **SC-003**: A developer can build a nested Container tree with AddChild and FindById in under 20 minutes
- **SC-004**: All 5 core type operations are covered by unit tests (90%+ line coverage)
- **SC-005**: State property notification from a worker thread delivers correctly on the main thread (verified by test)
- **SC-006**: Batch coalescing reduces multiple property changes to a single notification (verified by test)

## Assumptions

- Core types are value types (copyable, comparable) used throughout the framework
- Property<T> stores values inline (not heap-allocated) for cache efficiency
- Widget Invalidation flags (`needs_layout_`, `needs_draw_`) are internal implementation details
- Container uses FlexLayout internally (Yoga-based) — the Yoga integration comes from existing P1 dependency
- The state module directory (`src/framework/state/`) already exists as a BUILD stub from P1
- Tests use googletest and can run headless (no display required)
- Cross-thread Property notification uses a lock-free queue or mutex-protected queue internally
