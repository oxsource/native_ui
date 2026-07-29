# Data / Entity Model: Flexbox Layout Engine

**Date**: 2026-07-29

## Entity: FlexLayout

| Field | Type | Description |
|-------|------|-------------|
| `root_` | YGNodeRef | Root Yoga node for this layout |
| `children_` | vector\<YGNodeRef\> | Child Yoga nodes (not owned) |

**Methods**: `Measure(Size)`, `Arrange(vector<MeasureResult>&, Size)`, `SetChildren(vector<YGNodeRef>)`

**Lifecycle**: Created → Configure (tagged params) → SetChildren → Measure → Arrange → Destroy

## Entity: MeasureResult

| Field | Type | Description |
|-------|------|-------------|
| `size` | Size | Child's computed size (filled by Measure) |
| `position` | Point | Child's computed position (filled by Arrange) |

## Entity: Direction

| Value | Meaning |
|-------|---------|
| `kRow` | Horizontal layout, left to right |
| `kColumn` | Vertical layout, top to bottom |

## Entity: JustifyContent

| Value | Meaning |
|-------|---------|
| `kFlexStart` | Children packed at start |
| `kCenter` | Children centered |
| `kFlexEnd` | Children packed at end |
| `kSpaceBetween` | Even spacing, first/last at edges |
| `kSpaceAround` | Even spacing with half gaps at edges |

## Entity: AlignItems / AlignContent

| Value | Meaning |
|-------|---------|
| `kStretch` | Children stretch to cross-axis size |
| `kFlexStart` | Children packed at cross-axis start |
| `kCenter` | Children centered on cross-axis |
| `kFlexEnd` | Children packed at cross-axis end |

## Entity: FlexWrap

| Value | Meaning |
|-------|---------|
| `kNoWrap` | Single line, may overflow |
| `kWrap` | Multiple lines, wrap at container edge |
| `kWrapReverse` | Multiple lines, reverse direction |

## Entity: YGNodeRef (opaque)

| Role | Description |
|------|-------------|
| Creator | YGNodeNew() allocates; YGNodeFree() frees |
| Config | YGNodeStyleSet*(node, value) |
| Compute | YGNodeCalculateLayout(root, ...) |
| Read | YGNodeLayoutGet*(node) returns computed values |
| Lifetime | Managed by Container, not by FlexLayout |
