# Data / Entity Model: SVG Image Support

**Date**: 2026-07-30

## Entity: SVG Rasterizer (nanosvg)

| Input | SVG XML file (`.svg`) |
|-------|----------------------|
| **Parser** | `nsvgParseFromFile(filepath, "px", 96.0f)` |
| **Rasterizer** | `nsvgRasterize(img, 0, 0, width, height)` |
| **Output** | Raw RGBA pixel buffer (`unsigned char*`) |
| **Wrap** | `SkBitmap::installPixels()` → `SkImage::MakeFromBitmap()` |

**Pipeline**:
```
SVG file → nsvgParseFromFile → NSVGimage (shapes)
  → nsvgRasterize → RGBA buffer
    → SkBitmap → SkImage → native::ui::Image
```

## Entity: Image::FromFile (extended)

| Input | Behavior |
|-------|----------|
| `photo.png` | Skia `SkImage::MakeFromEncoded` (existing) |
| `photo.jpg` | Skia `SkImage::MakeFromEncoded` (existing) |
| `vector.svg` | nanosvg parse + rasterize (NEW) |
| `missing.xyz` | Returns nullptr |

## Entity: ImageWidget (ScaleType comparison)

| Card | Source | ScaleType | Purpose |
|------|--------|-----------|---------|
| 1 | `superdog.svg` | kCenter | SVG original size, no scaling |
| 2 | `superdog.svg` | kCenterInside | SVG scaled uniformly |
| 3 | `police.png` | kCenterCrop | PNG fill+crop |
| 4 | `police.png` | kCenterInside | PNG uniform scale |
| 5 | `police.png` | kFillXY | PNG stretch |

## Relationships

```
Glide::Load("photo.svg")
  → worker thread
    → Image::FromFile("photo.svg")
      → detect ".svg"
        → nsvgParseFromFile → NSVGimage
        → nsvgRasterize → RGBA buffer
        → SkBitmap → SkImage → Image
    → callback(main thread)
      → ImageWidget::loaded_image_ = image
      → RequestRedraw()
```

## Validation Rules

| Rule | Entity | Description |
|------|--------|-------------|
| SVG file missing returns nullptr | Image::FromFile | nanosvg returns null → Image* is null |
| SVG malformed XML returns nullptr | Image::FromFile | nanosvg parse fails → Image::FromFile returns nullptr |
| Rasterization respects target size | nanosvg | `width`/`height` params passed to nsvgRasterize match widget bounds |
| Glide worker thread not blocking | Glide | `std::async(std::launch::async, ...)` ensures main thread free |
| Example outputs valid PNG | image_gallery | `PngWriter::Write` confirms PNG header is valid |
