#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace native::ui {

struct Font {
  std::string family;  // "" = unset → default font
  int weight = 0;      // 100–900; 0 treated as 400
  float size = 0;      // >0; 0 treated as 16
};

struct FontManagerInternal;  // render-internal bridge (defined in .internal.h)

class FontManager {
public:
  static FontManager& Default();

  // Registers a font file under a family name (weight 100–900, default 400).
  // First successful registration becomes the implicit default font (FR-013).
  // Returns true on success; false + last_error() on missing/corrupt file (FR-006).
  bool RegisterFont(const std::string& family, const std::string& file_path,
                    int weight = 400);

  // Explicitly designates a registered family as the default (FR-014).
  // Returns false + last_error() if `family` is not registered.
  bool SetDefaultFont(const std::string& family);

  bool HasDefaultFont() const;
  const std::string& last_error() const;

  // Resets registry + cache + default (test reset, not thread-safe mid-draw).
  void Clear();

private:
  struct Entry {
    std::string family;
    int weight = 400;
    std::string path;
  };

  // Forward declaration of the render-internal accessor (defined in
  // font_manager_internal.h) so the public header stays Skia-free.
  friend struct FontManagerInternal;  // provides ResolveTypeface to Canvas

  // Opaque handle into the cache. Owns a heap-allocated
  // (sk_sp<SkTypeface>, sk_sp<SkData>) pair defined only in the .cc, so the
  // public header never leaks Skia types (render encapsulation rule).
  struct ResolvedFont {
    void* state = nullptr;  // SkTypefaceHolder*, owned by cache_
  };

  using FamilyKey = std::pair<std::string, int>;

  static FontManager& Instance();
  void ClearLocked();

  const ResolvedFont* Resolve(const std::string& family, int weight) const;
  const ResolvedFont* ResolveExactOrNearest(const std::string& family,
                                            int weight) const;
  const ResolvedFont* ResolveDefault(int weight) const;
  const ResolvedFont* ResolvePlatformDefault() const;
  void Evict(const std::string& family);

  mutable std::map<FamilyKey, ResolvedFont> cache_;
  std::map<std::string, std::vector<Entry>, std::less<>> registry_;
  std::string default_family_;  // "" = none
  mutable std::string last_error_;

  friend class Canvas;                        // Canvas calls Resolve
};

}  // namespace native::ui