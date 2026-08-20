#pragma once

#include "SkRefCnt.h"

#include "src/framework/render/font_manager.h"

class SkTypeface;

namespace native::ui {

// Internal bridge between FontManager (opaque cache state) and Canvas (which
// builds SkFont from a SkTypeface). Defined in font_manager.cc where the
// opaque TypefaceHolder payload is visible. Used only inside //src/framework/
// render — the public font_manager.h stays Skia-free.
//
// Returns a retained sk_sp<SkTypeface> for family+weight (weight 0 → 400,
// nearest registered variant, default/platform fallback). May return null only
// if NO usable typeface exists at all (e.g. platform has no default rasterizer
// and nothing is registered). Callers must not keep the returned pointer past
// a re-registration of that family (FR-010).
struct FontManagerInternal {
  static sk_sp<SkTypeface> ResolveTypeface(const Font& font);
};

}  // namespace native::ui