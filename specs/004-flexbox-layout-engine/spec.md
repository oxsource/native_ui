# Feature Specification: Flexbox Layout Engine

**Feature Branch**: `004-flexbox-layout-engine`

**Created**: 2026-07-29

**Status**: Draft

**Input**: User description: "Flexbox Layout Engine — Phase 4"

## User Scenarios & Testing

### User Story 1 - Layout Engine Computes Measure and Arrange (Priority: P1)

A developer uses `FlexLayout` to compute the layout of child elements within a parent container, first calculating the size of each child (`Measure`), then determining their final positions (`Arrange`).

**Why this priority**: Measure and Arrange are the core layout pipeline that every Container widget depends on. Without them, widgets cannot be positioned on screen.

**Independent Test**: A developer can call `Measure({800, 600})` on a FlexLayout with configured properties and children, receive correctly computed child sizes, then call `Arrange` to get child positions — all verified by unit tests.

**Acceptance Scenarios**:

1. **Given** a FlexLayout configured with Direction(kRow), **When** Measure and Arrange are called, **Then** children are positioned horizontally with their Yoga-computed sizes
2. **Given** a FlexLayout configured with Direction(kColumn), **When** Measure and Arrange are called, **Then** children are positioned vertically with correct stacking
3. **Given** a FlexLayout with Gap(8), **When** arranged, **Then** adjacent children have 8px spacing between them

---

### User Story 2 - Layout Supports All Flexbox Properties (Priority: P1)

A developer can configure a FlexLayout with any standard flexbox property — justify-content, align-items, flex-wrap, grow, shrink, basis, margin, padding — and verify the layout respects the settings.

**Why this priority**: The full flexbox specification is needed to build flexible, responsive UIs. Missing properties would limit the layout capability for downstream widgets.

**Independent Test**: Each flexbox property (justify, align, wrap, grow, shrink, gap, padding, margin) has a corresponding unit test that produces the expected layout output.

**Acceptance Scenarios**:

1. **Given** a FlexLayout with `JustifyContent(kCenter)`, **When** arranged, **Then** children are centered within the container
2. **Given** a FlexLayout with `AlignItems(kStretch)`, **When** arranged, **Then** children are stretched to the container's cross-axis size
3. **Given** a FlexLayout with `FlexWrap(kWrap)`, **When** children exceed container width, **Then** they wrap to the next line

---

### Edge Cases

- What happens when FlexLayout has zero children?
- What happens when the available size is zero or negative?
- What happens when gap exceeds the available space?
- What happens when flex-grow is set on all children with insufficient space?

## Requirements

### Functional Requirements

- **FR-001**: FlexLayout must support tagged-parameter construction accepting Direction, JustifyContent, AlignItems, FlexWrap, Gap, Padding, Margin tags
- **FR-002**: FlexLayout::Measure must return a vector of MeasureResult, each containing a Size
- **FR-003**: FlexLayout::Arrange must populate the position field of each MeasureResult
- **FR-004**: FlexLayout must internally wrap Yoga C API (YGNodeNew, YGNodeStyleSet*, YGNodeCalculateLayout, YGNodeLayoutGet*)
- **FR-005**: FlexLayout must manage YGNodeRef lifecycle — create on construction, free on destruction
- **FR-006**: FlexLayout must accept a `SetChildren(vector<YGNodeRef>)` call to register child Yoga nodes
- **FR-007**: FlexLayout must support flex-grow, flex-shrink, and flex-basis per child
- **FR-008: Must provide a public header exposing FlexLayout and MeasureResult

### Key Entities

- **FlexLayout**: A C++ wrapper over Yoga's YGNodeRef, providing tagged-parameter configuration, Measure, and Arrange
- **MeasureResult**: A struct containing the computed size and arranged position of a child element

## Success Criteria

### Measurable Outcomes

- **SC-001**: A developer can configure a FlexLayout with 3 flexbox properties and compute layout in under 5 minutes
- **SC-002**: All 8 flexbox properties (direction, justify, align, wrap, gap, padding, margin, grow) are covered by unit tests
- **SC-003**: FlexLayout handles empty children list without crash
- **SC-004**: FlexLayout handles zero-size container without crash

## Assumptions

- FlexLayout does not own the child Yoga nodes — they are managed by the Container
- FlexLayout is a main-thread-only API (not thread-safe)
- The Yoga library is already integrated as an external dependency from Phase 1
- Tag types (Direction, Gap, Padding, etc.) reuse the definitions from the Container implementation in Phase 3
- Padding and Margin are single-float values (uniform on all edges) for MVP
