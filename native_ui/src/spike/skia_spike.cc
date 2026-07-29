#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkPaint.h"
#include "SkGraphics.h"

#include "png_writer.h"

#include <cstdio>

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

    if (!WriteSkSurfaceToPNG(surface.get(), path)) {
        return 1;
    }

    std::printf("Skia spike passed — wrote %s\n", path);
    return 0;
}
