# Contract: FontManager — External Font Registration Interface

**Scope**: The developer-facing interface to register fonts by **file path** (`RegisterFont`), designate a **default font** (implicit first-registered + explicit `SetDefaultFont`), and the internal resolution rule (`Resolve`) that feeds text measurement and drawing. Effective on **all supported platforms** (Android 10+/API 29+, macOS, Linux) through one identical API. **Public header**: external consumers include `<native_ui/font.h>` (aggregated by `<native_ui/render.h>`).

## Public API

```cpp
namespace native::ui {

class FontManager {
public:
  static FontManager& Default();                 // process-wide singleton (like Glide::Default / Style::Default)

  // -- Family-name constants --
  // kDefaultFontFamily ("") = unset → framework default font (FR-013).
  // System names are suggested identifiers external code may reference; they
  // resolve through the normal registered-family/fallback path.
  static constexpr const char* kDefaultFontFamily = "";       // unset → default font
  static constexpr const char* kSansSerifFamily  = "sans-serif";
  static constexpr const char* kSerifFamily      = "serif";
  static constexpr const char* kMonospaceFamily  = "monospace";

  // -- Common font-size constants (logical px; default size is 16) --
  static constexpr float kFontSizeCaption    = 12.0f;
  static constexpr float kFontSizeBody       = 14.0f;
  static constexpr float kFontSizeBodyLarge  = 16.0f;  // default when unset
  static constexpr float kFontSizeTitle      = 20.0f;
  static constexpr float kFontSizeHeadline   = 24.0f;
  static constexpr float kFontSizeDisplay    = 32.0f;
  static constexpr float kFontSizeHero       = 48.0f;

  // -- Common font-weight constants (property range 100–900) --
  static constexpr int kFontWeightRegular = 400;
  static constexpr int kFontWeightMedium  = 500;
  static constexpr int kFontWeightBold    = 700;

  // -- Registration (spec FR-001/002) --
  // Registers a font file under a family name. weight ∈ [100,900], default 400.
  // Returns true on success (file parseable); false + last_error() on failure (FR-006).
  bool RegisterFont(const std::string& family,
                    const std::string& file_path,
                    int weight = 400);

  // -- Default font designation (spec FR-013/014) --
  // Implicit: the FIRST successful RegisterFont(...) becomes the default.
  // Explicit override: designates an already-registered family as the default.
  // Returns false (no-op + last_error()) if `family` is not registered yet.
  bool SetDefaultFont(const std::string& family);
  bool HasDefaultFont() const;

  // -- Observability --
  // Human-readable last error (empty on success); set by RegisterFont / SetDefaultFont.
  const std::string& last_error() const;

  void Clear();                                  // resets registry+cache+default (tests)
};

// Value descriptor passed to Canvas::MeasureText / Canvas::DrawText.
// Widgets build this from style(): family + weight from style, size from font_size.
struct Font {
  std::string family;   // "" = unset → default font
  int         weight;   // 100–900 (0 → treated as 400)
  float       size;     // >0, default 16.0f when 0
};

}  // namespace native::ui
```

## Canvas integration (consume side)

```cpp
class Canvas {
// New overloads — the ONLY public font paths. Existing scalar overload is kept
// (delegates to Font{size=font_size}) so debug_overlay.cc stays unchanged (FR-012).
public:
  Rect MeasureText(const std::string& text, const Font& font);
  void DrawText(const std::string& text, Point pos, const Paint& paint, const Font& font);
  void DrawText(const std::string& text, Point pos, const Paint& paint,
                float font_size);               // existing, delegates to Font{...}
};
```

`MeasureText` and `DrawText` MUST resolve through the same `FontManager::Default().Resolve(family, weight)` call so the typeface serving measurement equals the typeface serving drawing (FR-008).

## Resolution rule (spec FR-004/007/013)

`Resolve(family, weight)` priority:

| # | Condition | Result |
|---|-----------|--------|
| 1 | `family` empty or unset (`""`) | default font if `HasDefaultFont()`; else platform default typeface (FR-013/012) |
| 2 | `family` registered, exact `(family, weight)` | cached `ResolvedFont` for that entry (FR-003) |
| 3 | `family` registered, exact weight absent | nearest registered weight of that family (`min|Δweight|`, tie → lower); if family has exactly one variant, that one (FR-004) |
| 4 | `family` unknown/never registered | default font if `HasDefaultFont()`; else platform default (FR-007) |
| 5 | registered path missing/corrupt at load time | observable error in `last_error()`; **not** registered (RegisterFont returned false); callers resolve via rule 4 → default (FR-006) |

`weight <= 0` in a descriptor is normalized to 400 before matching.

## Default font state

- **Implicit**: set on the very first successful `RegisterFont(...)` (whichever family/weight) → `HasDefaultFont()` = true, default = that family's nearest-registered weight for that weight (FR-013).
- **Explicit**: `SetDefaultFont(family)` repoints default to that registered family; returns false if not yet registered (FR-014). Re-registering the default family's path refreshes the default's resolved typeface, designation persists (spec edge cases).
- No un-default path in v1 (explicit override only).

## Cache & memory (spec FR-010/011)

- Cache key `(family, weight)` → `ResolvedFont{SkData kept alive, sk_sp<SkTypeface>}`. **Cardinality invariant: `|cache| <= |registry|`, plus 1 default alias**; repeated `Resolve` hits the same slot → memory bounded over 10,000 resolutions (SC-004).
- Re-register `(family, weight)` evicts that slot; next resolve reloads the new path (FR-010).
- `Clear()` drops registry + cache + default (test-only reset, not thread-safe to call during draws).

## Threading

Registration and default designation are intended at **startup, before first draw** (spec Assumptions). Live re-registration mid-render is supported (cache refresh, FR-010) but the manager is **not** required to be thread-safe against *concurrent* registration + draw; v1 documents single-threaded registration / render-thread resolution.

## Platform manager construction (internal; not part of public API)

| Platform | Default SkFontMgr | Registered-file rasterizer |
|----------|-------------------|----------------------------|
| macOS (`__APPLE__`) | `SkFontMgr_New_CoreText(nullptr)` — preserved (FR-012) | same mgr, `makeFromData` (CoreText) |
| Android (`__ANDROID__`) | `SkFontMgr_New_Custom_Directory("/system/fonts")` | FreeType (`makeFromData`) |
| Linux / other | `SkFontMgr_New_Custom_Directory(<empty>::dir)` | FreeType (`makeFromData`) |

FreeType sources (`SkFontHost_FreeType*`, `SkTypeface_FreeType`, `SkFontMgr_custom*`) are compiled into Skia as part of this feature (prerequisite — currently excluded in `third_party/skia.BUILD:22-24`); the framework constructs managers explicitly (pattern precedent `canvas.cc:145-151`) rather than relying on Skia factory macros. Where a platform previously had no rasterizer (Android/Linux today → empty typeface), the registered path MUST now render real glyphs; the *unregistered* default path keeps today's behavior (no regression, FR-012).

## Error handling

- No exceptions (project rule). `RegisterFont`/`SetDefaultFont` return `bool`; `last_error()` carries a human-readable message (`kSuccess` empty). Failed loads never crash and never create/keep default entries (FR-006).
- `Resolve` never returns null (always yields at least the platform default typeface or the task-assigned no-op empty render where the platform previously had none); internal typeface handed to Skia, never exposed through public API.

## Verification contract (mapped to spec SC)

| Spec SC | Test hook |
|---------|-----------|
| SC-001 | `font_manager_test.cc`: register test font → `MeasureText("Hello", Font{family,400,24})` width > 0 (host, all 3 platforms compile). |
| SC-002 | register regular + bold files → `MeasureText` same string at weight 400 vs 700 widths differ, bold ≥ regular. |
| SC-003 | unknown family + missing-path register → falls back to default, no crash. |
| SC-004 | 10,000 `Resolve`/`MeasureText` loop → bounded residency (no monotonic growth). |
| SC-006 | first-registered becomes default → empty-family `MeasureText` > 0 and matches default family metrics. |
| Fr-012 | existing snapshot text/widget tests unchanged on all platforms. |