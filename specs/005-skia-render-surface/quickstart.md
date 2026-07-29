# Developer Quickstart: Skia Render Wrapper & Surface

## Build & Test

```bash
bazel build //src/framework/render //src/framework/surface
bazel test //tests:render_test //tests:surface_test
```

## Usage

```cpp
using namespace native::ui;

auto surface = Surface::Create(200, 200);
{
  Canvas canvas(*surface);
  Paint paint;
  paint.SetColor(Color::kRed).SetAntiAlias(true);
  canvas.DrawRect({10, 10, 100, 100}, paint);

  auto img = Image::FromFile("icon.png");
  canvas.DrawImage(*img, {120, 10, 80, 80});
}
surface->Flush();
```

## Key Conventions

- Canvas is RAII — create on stack, auto-restore on scope exit
- Paint is chainable — `paint.SetColor(...).SetAntiAlias(true)`
- Image decoding is lazy — deferred until DrawImage
- Only render/ and surface/ modules may `#include` Skia headers
