#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkPngEncoder.h"
#include "SkPaint.h"
#include "SkData.h"

#include <cstdio>

int main() {
    auto imageInfo = SkImageInfo::MakeN32Premul(200, 200);
    auto surface = SkSurfaces::Raster(imageInfo);
    if (!surface) {
        std::fprintf(stderr, "Failed to create SkSurface\n");
        return 1;
    }

    auto* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeXYWH(50, 50, 100, 100), paint);

    auto image = surface->makeImageSnapshot();
    if (!image) {
        std::fprintf(stderr, "Failed to snapshot image\n");
        return 1;
    }

    auto data = SkPngEncoder::Encode(nullptr, image.get(), {});
    if (!data) {
        std::fprintf(stderr, "Failed to encode PNG\n");
        return 1;
    }

    FILE* fp = std::fopen("skia_spike_output.png", "wb");
    if (!fp) {
        std::fprintf(stderr, "Failed to open output file\n");
        return 1;
    }
    std::fwrite(data->data(), 1, data->size(), fp);
    std::fclose(fp);

    std::printf("Skia spike passed — output written to skia_spike_output.png\n");
    return 0;
}
