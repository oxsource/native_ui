#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkImage.h"
#include "SkPaint.h"
#include "SkGraphics.h"

#include <cstdio>

int main() {
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

    auto image = surface->makeImageSnapshot();
    if (!image) {
        std::fprintf(stderr, "FAIL: makeImageSnapshot returned null\n");
        return 1;
    }

    uint32_t pixel = 0;
    SkImageInfo pixelInfo = SkImageInfo::MakeN32Premul(1, 1);
    if (!image->readPixels(pixelInfo, &pixel, 4, 75, 75)) {
        std::fprintf(stderr, "FAIL: readPixels failed\n");
        return 1;
    }

    std::printf("Skia spike passed — pixel at (75,75) = 0x%08x\n", pixel);
    return 0;
}
