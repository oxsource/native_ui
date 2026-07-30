#include "png_writer.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#pragma GCC diagnostic pop

#include <cstdio>
#include <vector>

namespace native::ui {

bool PngWriter::Write(SkSurface* surface, const char* path) {
  SkPixmap pixmap;
  if (!surface->peekPixels(&pixmap)) {
    std::fprintf(stderr, "FAIL: peekPixels failed\n");
    return false;
  }

  int w = pixmap.width(), h = pixmap.height();
  std::vector<uint8_t> rgb(w * h * 3);
  const auto* src = static_cast<const uint8_t*>(pixmap.addr());
  for (int i = 0; i < w * h; i++) {
    rgb[i * 3 + 0] = src[i * 4 + 0];
    rgb[i * 3 + 1] = src[i * 4 + 1];
    rgb[i * 3 + 2] = src[i * 4 + 2];
  }
  if (!stbi_write_png(path, w, h, 3, rgb.data(), 0)) {
    std::fprintf(stderr, "FAIL: stbi_write_png\n");
    return false;
  }
  return true;
}

}  // namespace native::ui
