# Public API Contract

**Purpose**: Define the public API surface that external consumers of native_ui can depend on.

## Entry Point

```cpp
#include "native_ui/native_ui.h"
```

Single umbrella header includes all public headers.

## Public Headers

| Header | Contents |
|--------|----------|
| `native_ui/native_ui_export.h` | `NATIVE_UI_API` export macro |
| `native_ui/core.h` | Re-exports: Rect, Point, Size, Color, EdgeInsets |
| `native_ui/layout.h` | Re-exports: FlexLayout, Direction, JustifyContent, etc. |
| `native_ui/render.h` | Re-exports: Canvas, Paint |
| `native_ui/surface.h` | Re-exports: Surface, BufferHandle |
| `native_ui/widgets.h` | Re-exports: Widget, Container, Text, Button, Image, Stack |
| `native_ui/event.h` | Re-exports: Event, HitTester |

## Visibility Rules

- Only `//src/framework/public:native_ui` has `//visibility:public`
- All internal modules use `visibility = ["//src/framework:__subpackages__", "//tests:__subpackages__"]`
- External consumers depend solely on `@native_ui//:native_ui`
- No external consumer may depend on internal modules directly

## Stability Guarantees

- Public API follows SemVer: breaking changes increment MAJOR version
- Public headers are stable within a MAJOR version
- Internal APIs (non-public headers) may change without notice
