# Data / Entity Model: Skia Render Wrapper & Surface

**Date**: 2026-07-29

## Entity: Surface

| Field | Type | Description |
|-------|------|-------------|
| `sk_surface_` | `sk_sp<SkSurface>` | Owned Skia surface (private) |

**Methods**: `Create(w,h)`, `CreateFromBuffer(HardwareBuffer)`, `Flush()`, `width()`, `height()`

## Entity: Canvas

| Field | Type | Description |
|-------|------|-------------|
| `surface_` | `Surface*` | Target surface (non-owning) |
| `save_count_` | int | SkCanvas save count at construction |

**Methods**: `DrawRect`, `DrawText`, `DrawPath`, `DrawImage`, `Save`, `Restore`, `ClipRect`, `Translate`

## Entity: Paint

| Field | Type | Description |
|-------|------|-------------|
| `color_` | Color | RGBA color |
| `anti_alias_` | bool | Anti-aliasing enabled |
| `stroke_width_` | float | Stroke width (0 = fill) |
| `style_` | PaintStyle | fill / stroke / fill-and-stroke |
| `alpha_` | uint8_t | Alpha override |

## Entity: Path

| Field | Type | Description |
|-------|------|-------------|
| `sk_path_` | `SkPath` | Underlying Skia path (private) |

**Methods**: `MoveTo`, `LineTo`, `CubicTo`, `Close`

## Entity: Image

| Field | Type | Description |
|-------|------|-------------|
| `data_` | `vector<uint8_t>` | Encoded data (for FromEncoded/FromFile) |
| `hardware_buffer_` | `HardwareBuffer` | Platform buffer (for FromBuffer) |
| `width_` | int | Image width (known after decode / from buffer) |
| `height_` | int | Image height |

**Factories**: `FromEncoded(data, size)`, `FromFile(path)`, `FromBuffer(HardwareBuffer)`

## Entity: HardwareBuffer

| Platform | Type | Wraps |
|----------|------|-------|
| macOS | `IOSurfaceRef` | `#if __APPLE__` |
| Linux | `int` (fd) | `#if __linux__` |
| Android | `AHardwareBuffer*` | `#if __ANDROID__` |

## Entity: SurfaceFactory

| Platform | Creates SkSurface from |
|----------|----------------------|
| macOS | `SkSurfaces::WrapIOSurface` |
| Linux | Custom DMA-BUF Skia surface |
| Android | `SkSurfaces::WrapAHardwareBuffer` |
