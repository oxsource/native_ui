#include "src/framework/render/font_manager_internal.h"

#include <cstdlib>
#include <limits>
#include <memory>

#include "SkData.h"
#include "SkFontMgr.h"
#include "SkStream.h"
#include "SkTypeface.h"

#if __APPLE__
#include "ports/SkFontMgr_mac_ct.h"
#else
#include "include/ports/SkFontMgr_directory.h"
#include "include/ports/SkFontMgr_empty.h"
#endif

namespace native::ui {

namespace {

const int kDefaultWeight = 400;

// Opaque cache payload: typeface + backing bytes, both kept alive for the
// cache slot lifetime (never exposed through the public header).
struct TypefaceHolder {
  sk_sp<SkTypeface> typeface;
  sk_sp<SkData> data;
};

int NormalizeWeight(int weight) {
  if (weight <= 0) return kDefaultWeight;
  if (weight < 100) return 100;
  if (weight > 900) return 900;
  return weight;
}

// Per-platform font manager (constructed once, process lifetime).
sk_sp<SkFontMgr>& PlatformFontMgr() {
  static sk_sp<SkFontMgr> mgr = [] {
#if __APPLE__
    return sk_sp<SkFontMgr>(SkFontMgr_New_CoreText(nullptr));
#elif defined(__ANDROID__)
    // Android: custom-directory manager over /system/fonts gives registered
    // files a FreeType backend and a usable default on-device (research Q3).
    return sk_sp<SkFontMgr>(
        SkFontMgr_New_Custom_Directory("/system/fonts"));
#else
    // Linux/other host: empty default manager (unregistered default renders
    // nothing, unchanged), registered files still render via FreeType.
    return sk_sp<SkFontMgr>(SkFontMgr_New_Custom_Empty());
#endif
  }();
  return mgr;
}

}  // namespace

FontManager& FontManager::Default() { return Instance(); }

FontManager& FontManager::Instance() {
  static FontManager mgr;
  return mgr;
}

void FontManager::ClearLocked() {
  for (const auto& [key, resolved] : cache_) {
    delete static_cast<TypefaceHolder*>(resolved.state);
  }
  cache_.clear();
  registry_.clear();
  default_family_.clear();
  last_error_.clear();
}

void FontManager::Clear() { ClearLocked(); }

bool FontManager::HasDefaultFont() const { return !default_family_.empty(); }

const std::string& FontManager::last_error() const { return last_error_; }

bool FontManager::RegisterFont(const std::string& family,
                               const std::string& file_path, int weight) {
  if (family.empty()) {
    last_error_ = "font family name must not be empty";
    return false;
  }
  const int w = NormalizeWeight(weight);

  auto data = SkData::MakeFromFileName(file_path.c_str());
  if (!data || data->size() == 0) {
    last_error_ = "cannot read font file (missing/unreadable/empty): " + file_path;
    return false;
  }

  // Probe-load now so corrupt files are rejected at register time (FR-006).
  sk_sp<SkTypeface> probe = PlatformFontMgr()->makeFromData(data);
  if (!probe) {
    last_error_ = "font file is not a loadable font format: " + file_path;
    return false;
  }

  // Replacement: drop the old cache slot for this family (FR-010).
  Evict(family);

  // Overwrite or append the registry entry.
  bool replaced = false;
  for (auto& e : registry_[family]) {
    if (e.weight == w) {
      e.path = file_path;
      replaced = true;
      break;
    }
  }
  if (!replaced) {
    registry_[family].push_back(Entry{family, w, file_path});
  }

  // First successful registration → implicit default (FR-013).
  if (default_family_.empty()) {
    default_family_ = family;
  }

  last_error_.clear();
  return true;
}

bool FontManager::SetDefaultFont(const std::string& family) {
  auto it = registry_.find(family);
  if (it == registry_.end() || it->second.empty()) {
    last_error_ = "cannot set default font: family not registered: " + family;
    return false;
  }
  default_family_ = family;
  last_error_.clear();
  return true;
}

void FontManager::Evict(const std::string& family) {
  for (auto it = cache_.begin(); it != cache_.end();) {
    if (it->first.first == family) {
      delete static_cast<TypefaceHolder*>(it->second.state);
      it = cache_.erase(it);
    } else {
      ++it;
    }
  }
}

// Creates the cached (or cache-hit) resolved font for a single registry entry.
const FontManager::ResolvedFont* FontManager::Resolve(
    const std::string& family, int weight) const {
  const int w = NormalizeWeight(weight);

  // Cache lookup (bounded: one slot per resolved key, refreshed on eviction).
  FamilyKey key{family, w};
  auto cached = cache_.find(key);
  if (cached != cache_.end()) {
    return cached->second.state ? &cached->second : nullptr;
  }

  auto it = registry_.find(family);
  const std::vector<Entry>* variants =
      (it != registry_.end() && !it->second.empty()) ? &it->second : nullptr;

  if (!variants) {
    // Unknown/unregistered family → default font (FR-007/FR-013), else
    // platform default (FR-012).
    if (!default_family_.empty() && default_family_ != family) {
      return Resolve(default_family_, w);
    }
    return ResolvePlatformDefault();
  }

  // Exact weight match (FR-003).
  const Entry* chosen = nullptr;
  for (const auto& e : *variants) {
    if (e.weight == w) {
      chosen = &e;
      break;
    }
  }

  // Nearest registered weight (FR-004): min |Δweight|, tie → lower.
  if (!chosen) {
    int best_delta = std::numeric_limits<int>::max();
    for (const auto& e : *variants) {
      int delta = std::abs(e.weight - w);
      if (delta < best_delta ||
          (delta == best_delta &&
           chosen != nullptr && e.weight < chosen->weight)) {
        best_delta = delta;
        chosen = &e;
      }
    }
  }

  if (!chosen) {
    return ResolvePlatformDefault();
  }

  const Entry& entry = *chosen;
  const FamilyKey entry_key{entry.family, entry.weight};

  // Reuse an existing cache slot if we just produced it in this call (e.g. the
  // family resolved to a different weight than requested).
  auto cached_entry = cache_.find(entry_key);
  if (cached_entry != cache_.end() && cached_entry->second.state) {
    return &cached_entry->second;
  }

  auto data = SkData::MakeFromFileName(entry.path.c_str());
  if (!data || data->size() == 0) {
    last_error_ = "registered font file no longer readable: " + entry.path;
    return ResolvePlatformDefault();
  }
  sk_sp<SkTypeface> typeface = PlatformFontMgr()->makeFromData(data);
  if (!typeface) {
    last_error_ = "registered font file no longer loadable: " + entry.path;
    return ResolvePlatformDefault();
  }

  auto* holder = new TypefaceHolder{std::move(typeface), std::move(data)};
  ResolvedFont resolved;
  resolved.state = holder;
  auto [slot, inserted] = cache_.emplace(entry_key, resolved);
  if (!inserted) {
    delete holder;
  }
  return &cache_.find(entry_key)->second;
}

const FontManager::ResolvedFont* FontManager::ResolveDefault(int weight) const {
  if (default_family_.empty()) return ResolvePlatformDefault();
  return Resolve(default_family_, weight);
}

const FontManager::ResolvedFont* FontManager::ResolvePlatformDefault() const {
  // Platform default typeface (e.g. CoreText default on macOS). Cached under
  // the reserved empty-family key so it is created once (memory bounded).
  FamilyKey key{"", kDefaultWeight};
  auto cached = cache_.find(key);
  if (cached != cache_.end() && cached->second.state) {
    return &cached->second;
  }

  sk_sp<SkTypeface> typeface = PlatformFontMgr()->matchFamilyStyle(
      nullptr, SkFontStyle());
  if (!typeface) {
    return nullptr;
  }
  auto* holder = new TypefaceHolder{std::move(typeface), nullptr};
  ResolvedFont resolved;
  resolved.state = holder;
  cache_.emplace(key, resolved);
  return &cache_.find(key)->second;
}

sk_sp<SkTypeface> FontManagerInternal::ResolveTypeface(const Font& font) {
  const FontManager::ResolvedFont* resolved =
      FontManager::Default().Resolve(font.family, font.weight);
  if (!resolved || !resolved->state) {
    return nullptr;
  }
  auto* holder = static_cast<TypefaceHolder*>(resolved->state);
  return holder->typeface;
}

}  // namespace native::ui