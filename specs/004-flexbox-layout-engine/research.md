# Research: Flexbox Layout Engine

**Date**: 2026-07-29

## Decisions

### Yoga C API Direct Wrapping

- **Decision**: Wrap Yoga's C API (YGNodeRef) directly in FlexLayout, rather than using a higher-level C++ Yoga wrapper.
- **Rationale**: Yoga's C API is stable, well-documented, and used by React Native. Direct wrapping keeps the abstraction thin and avoids an additional dependency layer. The FlexLayout class already mirrors Container's ProcessArg dispatch pattern from Phase 3.
- **Alternatives considered**: yoga-layout (JavaScript/WASM — rejected: incompatible with C++); facebook/yoga v2 C++ headers (rejected: v2 uses C++14/17 but exposes internals; v3 requires C++20)

### Tag Types Reuse from Container

- **Decision**: FlexLayout uses the same tag types (Direction, Padding, Gap, Margin) as Container from Phase 3, defined centrally in the layout module.
- **Rationale**: Container already defines Direction, Padding, Gap, Margin in container.h. FlexLayout should define its own canonical versions to avoid a dependency on widgets. Container can alias or include these.
- **Alternatives considered**: Sharing tag types via core module (rejected: tag types are layout-specific, not general enough for core); Defining in widgets only (rejected: layout should not depend on widgets)

### Children via SetChildren, Not Ownership

- **Decision**: FlexLayout does not own child Yoga nodes. Children are set via `SetChildren(std::vector<YGNodeRef>)` and the caller (Container) manages their lifecycle.
- **Rationale**: Container already creates and manages YGNodeRef per child. Duplicating ownership would complicate cleanup. FlexLayout's role is calculation only.
- **Alternatives considered**: FlexLayout owns + clones nodes (rejected: unnecessary allocation); Pass children every Measure call (rejected: ownership confusion)

### MeasureResult with Position Placeholder

- **Decision**: MeasureResult has both size and position. Measure fills size only; Arrange fills position. This enforces the two-phase protocol at the type level.
- **Rationale**: Clear separation of concerns — Measure computes sizes (Yoga forward pass), Arrange computes positions (Yoga backward pass). A single struct avoids separate arrays.
