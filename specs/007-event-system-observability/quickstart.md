# Developer Quickstart: Event System & Observability

## Build & Test

```bash
bazel build //src/framework/event //src/framework/widgets
bazel test //tests:event_test //tests:debug_overlay_test
```

## Usage

```cpp
using namespace native::ui;

// --- Create a widget tree ---
auto tree = Container(Direction(kColumn), Children{
    std::make_unique<Button>(Label("Click me"), OnClick([&] {
        // Handle click
    })),
    std::make_unique<Text>(Content("Hello")),
});

// --- Create EventHub and push a mock click ---
EventHub hub;
auto* root = tree.get();
hub.hit_tester().SetRoot(root);

MouseEvent click;
click.position = Point{50, 30};
click.button = MouseButton::kLeft;

DispatchResult result = hub.Push(click);
// result.status == kHandled (Button was at click position)
// result.target == pointer to the Button widget

// --- Push a click on empty area ---
click.position = Point{0, 0};
result = hub.Push(click);
// result.status == kNoTarget

// --- Register an event filter ---
hub.AddFilter([](const MouseEvent& ev) -> bool {
    return ev.position.x > 10;  // reject events at x <= 10
});

// --- DebugOverlay ---
DebugOverlay overlay;
overlay.set_fps(60);
overlay.set_breadcrumb("root > Container#main > Button#ok");
overlay.Toggle();  // show overlay
tree->AddChild(std::make_unique<DebugOverlay>(overlay));
```

## Key Conventions

- Events are externally injected via `EventHub::Push()` — no platform event loop in the framework
- All tests use mock `MouseEvent`/`KeyEvent` values — no platform dependency
- Hit testing respects Stack z-order (topmost child tested first)
- Filters run before dispatch — return `false` to reject an event (kRejected)
- Widgets opt into event handling by overriding `OnMouseEvent` / `OnKeyEvent`
- DebugOverlay is compiled out in release builds (`#ifndef NDEBUG`)
- Default toggle shortcut: F12 (handled via OnKeyEvent)
