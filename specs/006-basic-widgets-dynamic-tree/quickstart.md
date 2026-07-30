# Developer Quickstart: Basic Widgets & Dynamic Tree

## Build & Test

```bash
bazel build //src/framework/widgets
bazel test //tests:widgets_test
```

## Usage

```cpp
using namespace native::ui;

// --- Counter with data binding ---
class CounterState : public State {
public:
  Property<int> count{this};
};

auto state = std::make_shared<CounterState>();

auto tree = Container(
    Direction(kColumn),
    Gap(8),
    Padding(16),
    Children{
        std::make_unique<Text>(Content("Count: 0"), Id("label")),
        std::make_unique<Button>(Label("Increment"), OnClick([&] {
            state->count = state->count.value() + 1;
        })),
    }
);

// Watch State property → auto redraw on change
tree->Watch(state->count);
state->count = 42;  // triggers RequestRedraw → label updates
```

## Stack Example

```cpp
auto stack = Stack(
    Children{
        std::make_unique<ImageWidget>(ImagePath("background.png")),
        std::make_unique<Text>(Content("Overlay"), FontSize(24), Color(Color::kWhite)),
    }
);
```

## ExternalImage Example

```cpp
using namespace native::ui;

// --- ExternalImage with camera buffer ---
auto state = std::make_shared<BufferState>();
auto ext_image = std::make_unique<ExternalImage>(Id("camera"));

// Bind to hardware buffer property for per-frame updates
ext_image->Watch(state->frame_buffer);
state->frame_buffer = HardwareBuffer::FromIOSurface(new_frame);
// → triggers RequestRedraw automatically

// Or set buffer directly:
ext_image->SetBuffer(HardwareBuffer::FromIOSurface(next_frame));
```

## Key Conventions

- All widgets support `Id(...)` for `FindById` lookup
- `Watch(Property<T>&)` on a widget subscribes to State changes — redraws automatically
- Button hit testing is self-contained — call `HitTest(Point)` to check bounds
- Image handles missing files gracefully (no crash, no render)
- Stack draws children in vector order (index 0 = bottom, N = top)
- All container widgets trigger `RequestLayout()` on `AddChild`/`RemoveChild`
