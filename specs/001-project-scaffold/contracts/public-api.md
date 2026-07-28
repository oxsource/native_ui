# Public API Contract: native_ui

**Date**: 2026-07-28

## Bazel Dependency

External projects consume native_ui via Bazel:

```python
# In WORKSPACE
http_archive(
    name = "native_ui",
    urls = ["<url_to_archive>"],
    sha256 = "<sha256>",
)

# In BUILD.bazel
cc_library(
    name = "my_app",
    deps = ["@native_ui//:native_ui"],
)
```

## Public Target: `//:native_ui`

- **Type**: `alias` → `//src/framework/public:native_ui`
- **Visibility**: `//visibility:public`
- **Provides**: Umbrella header `native_ui/native_ui.h`

## Umbrella Header Interface

```cpp
#include "native_ui/native_ui.h"
```

The umbrella header includes all public sub-headers:
- `native_ui/native_ui_export.h` — `NATIVE_UI_API` visibility macro
- `native_ui/core.h` — Core types (Rect, Point, Size, Color, EdgeInsets)
- `native_ui/layout.h` — FlexLayout
- `native_ui/render.h` — Canvas, Paint, Path
- `native_ui/surface.h` — PlatformSurface, BufferHandle
- `native_ui/widgets.h` — Widget, Container, Text, Button, Image, Stack
- `native_ui/event.h` — Event, HitTester

## Export Macro

```cpp
NATIVE_UI_API
```

- On Apple platforms: `__attribute__((visibility("default")))`
- On Linux: `__attribute__((visibility("default")))`
- In static builds: empty (no-op)

## Shared Library Target (future)

- **Target**: `//src/framework/public:native_ui_shared`
- **Type**: `cc_binary(linkshared=True, linkstatic=True)`
- **Enabled by**: Defining `NATIVE_UI_SHARED_LIBRARY` during compilation
