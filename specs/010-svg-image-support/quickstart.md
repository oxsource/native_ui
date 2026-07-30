# Developer Quickstart: SVG Image Support

## Build & Run Image Gallery

```bash
# Fetch nanosvg headers (one-time)
bash scripts/fetch_nanosvg.sh

# Build the image gallery example
bazel build //examples:image_gallery

# Run it
bazel run //examples:image_gallery
# Output: /tmp/image_gallery.png
```

## How It Works

```cpp
// 1. Image::FromFile detects .svg and calls nanosvg
auto img = Image::FromFile("assets/photo/superdog.svg");

// 2. Glide loads PNG and SVG asynchronously on worker thread
Glide::SetDefault(new DefaultGlide());
// ImageURI triggers Glide::Load internally

// 3. ImageWidget uses ImageURI for both formats
ImageWidget(ImageURI("assets/photo/police.png"), ScaleType(kCenterCrop));
ImageWidget(ImageURI("assets/photo/superdog.svg"), ScaleType(kCenterInside));
```

## Gallery Cards

| Card | Image | ScaleType | Description |
|------|-------|-----------|-------------|
| 1 | superdog.svg | kCenter | SVG original size |
| 2 | superdog.svg | kCenterInside | SVG scaled uniformly |
| 3 | police.png | kCenterCrop | PNG fill + crop |
| 4 | police.png | kCenterInside | PNG uniform scale |
| 5 | police.png | kFillXY | PNG stretched |

## Key Files

- `third_party/nanosvg/nanosvg.h` — SVG parser
- `third_party/nanosvg/nanosvgrast.h` — SVG rasterizer
- `src/framework/render/image.cc` — SVG detection and nanosvg integration
- `examples/image_gallery.cc` — Gallery example with 5 cards
- `assets/photo/police.png` — Test PNG
- `assets/photo/superdog.svg` — Test SVG
