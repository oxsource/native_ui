#pragma once

// Spike-only helper: save an SkSurface to a PNG. The Skia spikes are intentionally
// Skia-exposed (special case) and own raw sk_sp<SkSurface>, so they use this local
// writer instead of the framework's Surface::Dump.

#include <cstdint>
#include <vector>

#include "SkSurface.h"
#include "stb_image_write.h"

inline bool WritePng(SkSurface* surface, const char* path) {
  if (!surface || !path) return false;
  SkPixmap pm;
  if (!surface->peekPixels(&pm)) return false;
  int w = pm.width(), h = pm.height();
  std::vector<uint8_t> rgb(static_cast<size_t>(w) * h * 3);
  const auto* src = static_cast<const uint8_t*>(pm.addr());
  for (int i = 0; i < w * h; i++) {
    rgb[i * 3 + 0] = src[i * 4 + 0];
    rgb[i * 3 + 1] = src[i * 4 + 1];
    rgb[i * 3 + 2] = src[i * 4 + 2];
  }
  return stbi_write_png(path, w, h, 3, rgb.data(), 0) != 0;
}
