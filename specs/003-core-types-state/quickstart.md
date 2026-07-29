# Developer Quickstart: Core Types & Widget Foundation

## Build

```bash
# Build core module
bazel build //src/framework/core

# Build all Phase 3 modules
bazel build //src/framework/core //src/framework/viewmodel //src/framework/widgets

# Run tests
bazel test //tests:core_test
bazel test //tests:state_test
bazel test //tests:widget_test
```

## Code Layout

```text
native_ui/src/framework/
├── core/                # Rect.h, Point.h, Size.h, Color.h, EdgeInsets.h
├── viewmodel/           # state.h, state.cc, property.h
└── widgets/             # widget.h, widget.cc, container.h, container.cc

native_ui/tests/
├── core_test.cc
├── state_test.cc
├── widget_test.cc
└── integration/
    └── container_layout_test.cc
```

## Key Conventions

- Core types are header-only (`#pragma once`, constexpr where possible)
- `namespace native::ui { }` for all code
- `State` uses `Property<T>` members — assign via `state->count = 42`
- `Widget::Watch(StateProperty)` to subscribe to data changes
- `Container` tagged-parameter: `Direction(kRow), Gap(8), Padding(12), Children{...}`
- No exceptions — use assertions for programming errors
- All code must compile with C++17 and Bazel
