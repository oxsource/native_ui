# Developer Quickstart

**Date**: 2026-07-29

## Prerequisites

- Bazel 6.5.0 (pinned by `.bazelversion`)
- Xcode Command Line Tools (macOS)
- C++17 compatible compiler

## Build

```bash
# Build everything
bazel build //...

# Build a specific target
bazel build //src/framework/public:native_ui

# Run spikes
bazel run //src/spike:skia_spike
bazel run //src/spike:yoga_spike
```

## Test

```bash
# Run all tests
bazel test //...

# Run specific test
bazel test //tests:infra_test
```

## Project Structure

```
native_ui/
├── doc/                          # Architecture docs, API contracts
│   ├── architecture/             # Module deps, error handling, memory, lifecycle
│   └── api/                     # Widget, layout, render, event contracts
├── platforms/                    # Bazel platform definitions
├── third_party/                  # Third-party BUILD wrappers
├── src/
│   └── framework/
│       ├── core/                 # Rect, Point, Size, Color, EdgeInsets
│       ├── layout/              # FlexLayout (Yoga wrapper)
│       ├── render/              # Canvas, Paint, Path
│       ├── surface/             # PlatformSurface, BufferHandle
│       ├── widgets/             # Widget, Container, Text, Button, Image, Stack
│       ├── event/               # HitTester, Event dispatch
│       └── public/              # Umbrella header, export macro
├── spike/                        # Integration spike binaries
├── tests/                        # Unit and integration tests
├── examples/                     # Example consumer projects
└── spec/                         # Feature specifications
```

## Key Conventions

- **Google C++ Style**: 2-space indent, 80-col width, snake_case files/vars, PascalCase types/functions
- **Conventional Commits**: `feat(core): add Rect base type`
- **Tagged Parameters**: `Container(Direction(kRow), Padding(16), Children{...})`
- **Skia Isolation**: Only `render/` and `surface/` may depend on Skia
