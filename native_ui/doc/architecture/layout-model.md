# Layout Model

The framework uses **Yoga** (flexbox) as its layout engine. Every `Container` manages a yoga node tree; leaf widgets (Text, Button, ImageWidget) are yoga children of their parent Container.

## Default Flex Behaviour

`Container::AddChild` sets every child node with:

```
flexGrow: 1
flexShrink: 1
```

This means **by default** all children share available space equally. No explicit width/height is needed for flex distribution.

## Explicit Width / Height

Widgets can declare explicit dimensions via `Width` / `Height` tags:

```cpp
auto img = std::make_unique<ImageWidget>(
    ImageURI{"photo.svg"},
    Width{120}, Height{320},   // explicit size
    ScaleMode::kCenter);
```

`Container::PrepareLayout` propagates these to the yoga node **and disables `flexGrow`**:

```
Width{120}  → yoga width  = 120, flexGrow = 0
Height{320} → yoga height = 320, flexGrow = 0
```

**Result**: the widget gets exactly 120 × 320 px — flex will not stretch it.

## The Rule

> **In a flex layout, do not set width / height on a widget if you expect flex to distribute space for it. Conversely, if you set an explicit size, flex will not participate.**

| Use case | Set Width/Height? | Behaviour |
|----------|-------------------|-----------|
| Button shares space equally | `flexGrow: 1` (default) | No Width/Height set → flex distributes |
| ImageWidget must be 120×320 | `Width{120}, Height{320}` | Explicit size → flexGrow disabled, exact size |
| Text fills row width | `Width{200}` only | Width fixed, height flex-based |
| Card with fixed children | Width/Height on children | Children get exact sizes, card stretches by flex |

## Under The Hood

```cpp
// AddChild (default flex)
YGNodeStyleSetFlexGrow(child_node, 1);   // share space
YGNodeStyleSetFlexShrink(child_node, 1);

// PrepareLayout (when Width/Height tag present)
float cw = child->style().width();    // e.g. 120
float ch = child->style().height();   // e.g. 320
if (cw > 0) YGNodeStyleSetWidth(child_node, cw);
if (ch > 0) {
  YGNodeStyleSetHeight(child_node, ch);
  YGNodeStyleSetFlexGrow(child_node, 0);   // FIXED SIZE, no flex
}
```

## Historical Note

Previously, explicit dimensions were **not** propagated to yoga. Widgets with `Width{120}` relied purely on flex to determine their final size — the `Width` tag only affected rendering, not layout. This caused two problems:

1. `Height{320}` on an ImageWidget had no effect (the widget was flex-stretched to whatever the container allocated).
2. Center-aligned containers (`AlignItems::kCenter`) gave child widgets zero width because yoga had no explicit width to work with.

The current design fixes both: explicit dimensions are honoured, and widgets without them keep their flex behaviour exactly as before.
