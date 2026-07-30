# Public API Contract (Hello World Consumer View)

**Purpose**: Document the public API surface that the Hello World example uses. This defines the contract between the framework and its consumers.

## Included Headers

```cpp
#include "native_ui/core.h"       // Rect, Point, Size, Color
#include "native_ui/widgets.h"    // Widget, Container, Text, Button, Stack, Id, Content, Label, OnClick, Direction, Padding, Gap
#include "native_ui/render.h"     // Canvas, Paint, Path
#include "native_ui/surface.h"    // Surface
#include "native_ui/viewmodel.h"  // State, Property<T>
#include "native_ui/event.h"      // EventHub, MouseEvent, DispatchResult
```

## Example Pipeline

```cpp
// 1. State with data binding
class AppState : public native::ui::State {
public:
  native::ui::Property<int> count{this};
};

// 2. Widget tree construction (tagged parameters)
auto container = native::ui::Container(
    native::ui::Direction(native::ui::Direction::kColumn),
    native::ui::Padding(16),
    native::ui::Gap(8),
    native::ui::Children{
        std::make_unique<native::ui::Text>(
            native::ui::Content("Count: 0"),
            native::ui::Id("label")),
        std::make_unique<native::ui::Button>(
            native::ui::Label("Increment"),
            native::ui::OnClick([&] { state->count = state->count.value() + 1; })),
    }
);

// 3. Data binding
container->Watch(state->count);

// 4. Layout
container->Measure({800, 600});
container->Arrange({800, 600});

// 5. Render
auto surface = native::ui::Surface::Create(800, 600);
{
    native::ui::Canvas canvas(*surface);
    container->Draw(canvas);
}
surface->Flush();

// 6. Encode to PNG (not in public API — use Skia directly or via helper)
```

## Platform Assumptions

- Canvas is created from an off-screen Surface (no window required)
- EventHub is optional — example can use direct State mutation to demonstrate data binding
- Layout is explicitly triggered by the consumer (Measure + Arrange)
- PNG encoding uses Skia's `SkSurface::makeImageSnapshot()->encodeToData()` — accessed via internal helper or example-specific code
