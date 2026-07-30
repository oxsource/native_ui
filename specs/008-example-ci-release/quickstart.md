# Developer Quickstart: Example, CI Polish & Release

## Build & Run Hello World

```bash
bazel build //examples:hello_world
bazel run //examples:hello_world
# Output: hello_world_output.png in current directory
```

## Build Shared Library

```bash
bash scripts/build_shared.sh
# Output: dist/libnative_ui_shared.dylib (macOS) or .so (Linux)
```

## Run All Tests

```bash
bazel test //...
```

## CI Pipeline

- `ci.yml`: Runs on push to main — `bazel build //...`, `bazel test //...`, visibility guard
- `pr.yml`: Runs on PRs — same checks + mandatory review
- `release.yml`: Runs on version tags — builds shared lib, creates GitHub Release

## Example Structure

```cpp
using namespace native::ui;

// State
class AppState : public State {
public:
  Property<int> count{this};
};

auto state = std::make_shared<AppState>();

// Widget tree
auto tree = Container(Direction(kColumn), Padding(16), Gap(8),
    Children{
        std::make_unique<Text>(Content("Count: 0"), Id("label")),
        std::make_unique<Button>(Label("Increment"),
            OnClick([&] { state->count = state->count.value() + 1; })),
    });

// Data binding + layout
tree->Watch(state->count);
tree->Measure({800, 600});
tree->Arrange({800, 600});

// Render to surface
auto surface = Surface::Create(800, 600);
{ Canvas canvas(*surface); tree->Draw(canvas); }
surface->Flush();

// Update and redraw
state->count = 5;  // triggers RequestRedraw
tree->Measure({800, 600});
tree->Arrange({800, 600});
{ Canvas canvas(*surface); tree->Draw(canvas); }
surface->Flush();
```

## Key Conventions

- Examples use only public API headers (`native_ui/...`)
- PNG encoding uses Skia utilities (accessed via third_party or internal helper)
- Event system integration is optional for data binding demo (direct State mutation suffices)
- CI matrix includes both macOS ARM64 and Linux x86_64
- Shared library target is `//src/framework/public:native_ui_shared`
