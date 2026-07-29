#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkImage.h"
#include "SkPaint.h"
#include "SkGraphics.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <cstdio>
#include <vector>

int main(int argc, char** argv) {
    const char* path = argc > 1 ? argv[1] : "/tmp/output.png";
    SkGraphics::Init();
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(200, 200));
    if (!surface) {
        std::fprintf(stderr, "FAIL: SkSurface::Raster returned null\n");
        return 1;
    }

    auto* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeXYWH(50, 50, 100, 100), paint);

    SkPixmap pixmap;
    if (!surface->peekPixels(&pixmap)) {
        std::fprintf(stderr, "FAIL: peekPixels failed\n");
        return 1;
    }

    int w = pixmap.width(), h = pixmap.height();
    std::vector<uint8_t> rgb(w * h * 3);
    const auto* src = static_cast<const uint8_t*>(pixmap.addr());
    // macOS kN32 = kRGBA_8888: byte[0]=R, byte[1]=G, byte[2]=B, byte[3]=A
    for (int i = 0; i < w * h; i++) {
        rgb[i * 3 + 0] = src[i * 4 + 0]; // R
        rgb[i * 3 + 1] = src[i * 4 + 1]; // G
        rgb[i * 3 + 2] = src[i * 4 + 2]; // B
    }
    if (!stbi_write_png(path, w, h, 3, rgb.data(), 0)) {
        std::fprintf(stderr, "FAIL: stbi_write_png\n");
        return 1;
    }

    std::printf("Skia spike passed — wrote %s\n", path);
    return 0;
}
