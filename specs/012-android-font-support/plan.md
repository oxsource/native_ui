# Implementation Plan: External Font Registration Interface

**Branch**: `012-android-font-support` | **Date**: 2026-08-20 | **Spec**: [spec.md](spec.md)

**Input**: Feature specification from `specs/012-android-font-support/spec.md`

## Summary

Provide a developer-facing interface to set fonts by external file path: `FontManager::RegisterFont(family, path, weight)` registers a `.ttf`/`.otf` file under a named family, the **first successful registration becomes the implicit default font** (overridable via `SetDefaultFont(family)`), and `FontFamily(...)`/`FontWeight(...)`/`FontSize(...)` on `Text`/`Button` finally take effect in rendering — today they are collected into `style_` but ignored, and on **Android/Linux text is invisible** because the Skia build compiles no font rasterizer (`SkFont(nullptr,…)` → `SkTypeface::MakeEmpty()`). The feature compiles Skia's FreeType rasterizer + a custom-directory font manager, adds a `render/font_manager.*` module with explicit per-platform `SkFontMgr` construction (CoreText on macOS preserved), routes `Canvas` through `MeasureText`/`DrawText(Font)` so measuring and drawing share one resolved typeface (FR-008), and verifies via host unit tests (metrics: non-empty bounds, bold≠regular, fallback no-crash, 10k-resolution bounded memory) plus an Android device demo (`examples/font_demo.cc` → PNG). All-platform parity; macOS/Linux unregistered default behavior unchanged (FR-012).

## Technical Context

**Language/Version**: C++17 (repo standard)

**Primary Dependencies**: render (Add `FontManager`/`Font` to `src/framework/render/`), widget style (`style_` family/weight/size), Skia (re-enable FreeType ports + `sk_sp<SkTypeface>`), **new third-party `freetype` http_archive** (prerequisite — not currently present), Android NDK (`__ANDROID__` path only). No JNI.

**Storage**: N/A (font files read at register time into memory; PNG out for device demo verification)

**Testing**: Bazel `cc_test` — new `tests/font_manager_test.cc` (register→measure bounds>0; regular-vs-bold metric diff; unknown family + missing/corrupt file → default fallback no-crash; 10,000 resolve loop memory-bound; first-registered→default; `SetDefaultFont` override) on host (macOS/Linux). Android device `examples/font_demo.cc` → register pushed font → draw Text → `Surface::Dump` PNG → pull via adb (mirrors feature 011 device loop).

**Target Platform**: Android 10+ (API 29+, sole implemented system-font path via `/system/fonts` custom dir mgr), macOS ARM64 host (CoreText preserved), Linux host (custom dir mgr). Interface identical on all three (FR-005).

**Project Type**: C++ library (framework render + widget layer) + host unit tests + Android demo.

**Performance Goals**: lazy resolution, cache bounded by registry; 10,000 successive resolutions produce no measurable growth (FR-011/SC-004); no per-frame file reload (SC "live updates").

**Constraints**: C++17, no exceptions, no Skia leakage in public API (widgets stay Skia-free; `Canvas` encapsulates). No new framework bundling of fonts (developer supplies paths; tests vendor 3 small font assets). FreeType compiled for all platforms (dead-stripped on macOS where CoreText is used). Re-registration supported; concurrent register+draw not required thread-safe (documented). Existing scalar `Canvas::DrawText(text,pos,paint,font_size)` kept for `debug_overlay` (FR-012).

**Scale/Scope**: 2 new source files (`render/font_manager.h/.cc`), 1 header-only `Font` descriptor (in `font_manager.h`), modifications to `render/canvas.h/.cc`, `widgets/text.cc`, `widgets/button.cc`, `third_party/skia.BUILD` (un-exclude FreeType/custom ports), `native_ui_deps.bzl` (add freetype), a few `BUILD.bazel` updates, 1 new test file, 1 new example, vendored test fonts, 1 make target. No new runtime deps beyond vendored FreeType.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Constitution file contains a placeholder template — no binding principles defined. **All gates PASS.**

## Project Structure

### Documentation (this feature)

```text
specs/012-android-font-support/
├── spec.md              # Feature specification (incl. Cliarification for default font)
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output — Q1..Q7 (rasterizer, fallback, default font)
├── data-model.md        # Phase 1 output — FontManager registry/cache/default entities
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output
│   └── font-manager.md  # FontManager::RegisterFont/SetDefaultFont/Resolve + Canvas integration
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code

```text
native_ui/third_party/
├── skia.BUILD                      # MODIFY — un-exclude FreeType/custom ports:
│                                   #   src/ports/SkFontHost_FreeType*.cpp / SkTypeface_FreeType.cpp
│                                   #   src/ports/SkFontMgr_custom*.cpp; keep mac_ct as-is
│                                   # add cc_library dep "@freetype//:freetype" + include dirs,
│                                   # define SK_FREETYPE_MINIMUM_RUNTIME_VERSION/... if required by ports
└── BUILD.bazel                     # (no change)

native_ui/native_ui_deps.bzl        # MODIFY — add freetype http_archive (e.g. 2.13.2) + BUILD file

native_ui/src/framework/render/
├── font_manager.h / font_manager.cc   # NEW — FontManager singleton: RegisterFont/SetDefaultFont/
│                                      #   HasDefaultFont/last_error/Clear; Font{M;} descriptor;
│                                      #   Resolve(family,weight)→ResolvedFont; per-platform
│                                      #   SkFontMgr construction (CoreText/CustomDirectory); cache
├── canvas.h / canvas.cc               # MODIFY — add MeasureText(text,Font)→Rect, DrawText(text,pos,
│                                      #   paint,Font); scalar DrawText delegates→Font; resolve via
│                                      #   FontManager (measure+draw same typeface)
└── BUILD.bazel                        # MODIFY — new cc_library render/font_manager (dep skia, core);
                                       #   render already exposes *.h/*.cc glob

native_ui/src/framework/widgets/
├── text.cc                            # MODIFY — build Font{family,weight,size} from style(); use
│                                      #   canvas.MeasureText/DrawText; remove #if __APPLE__ blocks
├── button.cc                          # MODIFY — same
└── text.h                             # (no change — tags already exist)

native_ui/tests/
├── font_manager_test.cc               # NEW — host metrics tests (SC-001/002/003/004/006, FR-012)
├── widgets_test.cc                    # MODIFY — add registered-family render assertions (no-crash + bounds)
├── assets/fonts/                      # NEW — vendored Roboto-Regular/Bold + display font (Apache-2.0,
│                                      #   from pinned Skia resources) for host measurability
└── BUILD.bazel                        # MODIFY — add font_manager_test target; data=assets/fonts

native_ui/examples/
├── font_demo.cc                       # NEW — Android device demo: RegisterFont("/data/local/tmp/....ttf")
│                                      #   → Text(FontFamily...) → draw → Surface::Dump PNG (FR-005
│                                      #   on-device proof; mirrors external_image_demo loop)
└── BUILD.bazel                        # MODIFY — add font_demo cc_binary; data: font asset

native_ui/mk/                          # MODIFY — add android-font-demo targets reusing android_build.sh/
                                       #   android_demo.sh pattern (register pruned to push font + pull PNG)
native_ui/tests/integration/BUILD.bazel # (optional) no change unless adding integration case
```

**Structure Decision**: The new font capability lives in `render/` because it owns all Skia encapsulation (per `surface.h:38-39` "Skia fully encapsulated", and the render contract `specs/002-architecture-engineering-design/contracts/render-contract.md:196`). Widgets (`text.cc`, `button.cc`) consume only `Canvas::MeasureText/DrawText(Font)`. The `FontManager` follows the existing singleton precedents (`Glide::Default()`, `Style::SetDefault`). FreeType/Android specifics stay inside Skia + `font_manager.cc` behind `#if __ANDROID__`; non-Android keeps CoreText. The Android demo lives under `examples/` and reuses the feature-011 device tooling (`android_demo.sh` family).

## Design

### 1. `render/font_manager.h/.cc` — FontManager + Font descriptor (NEW)

```cpp
namespace native::ui {

struct Font { std::string family; int weight; float size; };   // weight 0→400, size 0→16

class FontManager {
public:
  static FontManager& Default();
  bool RegisterFont(const std::string& family, const std::string& file_path,
                    int weight = 400);
  bool SetDefaultFont(const std::string& family);
  bool HasDefaultFont() const;
  const std::string& last_error() const;
  void Clear();

  // internal (used by Canvas)
  struct ResolvedFont;
  const ResolvedFont* Resolve(const std::string& family, int weight) const;

private:
  std::map<std::string, std::vector<Entry>, std::less<>> registry_;  // family → weight variants
  mutable std::map<std::pair<std::string,int>, ResolvedFont> cache_;
  std::string default_family_;        // "" = none
  sk_sp<SkFontMgr> mgr_;              // per-platform, lazily built
  std::string last_error_;
};
}  // namespace native::ui
```

- **Resolve priority** (FR-004/007/013): empty family → default (else platform default); exact (family,weight) → cached; family with no exact weight → nearest registered weight; unknown family → default (else platform default); load failure at first resolve → marks error, returns default, cache stays empty.
- **Registry replaces on same (family,weight)** — eviction of that cache slot (FR-010). `|cache| ≤ |registry| + 1 default` (FR-011).
- **Platform managers** (Q2/Q3): `#if __APPLE__` → `SkFontMgr_New_CoreText(nullptr)` (preserved); `#elif __ANDROID__` → `SkFontMgr_New_Custom_Directory("/system/fonts")` + FreeType; `#else` → `SkFontMgr_New_Custom_Directory(<default-empty dir>)` + FreeType. Registered fonts load via `mgr_->makeFromData(SkData::MakeFromFileName(path))`; failures set `last_error_`.
- **First successful registration** records `default_family_` (FR-013); `SetDefaultFont` overrides it, validating the family exists (FR-014); `SetDefaultFont` on unknown family → false + error, default unchanged.

### 2. `Canvas` — MeasureText + DrawText(Font) (MODIFY canvas.h/.cc)

```cpp
Rect Canvas::MeasureText(const std::string& text, const Font& font);
void Canvas::DrawText(const std::string& text, Point pos, const Paint& paint, const Font& font);
// existing scalar:
void Canvas::DrawText(const std::string& text, Point pos, const Paint& paint, float font_size)
    { DrawText(text, pos, paint, Font{/*family=""*/, /*weight=*/0, font_size}); }
```

- `SkFont` is built once per call from `FontManager::Default().Resolve(font.family, font.weight)->typeface` and `font.size`. Both Measure and Draw go through the same `Resolve` → identical typeface (FR-008).
- `MeasureText` returns the bounds `Rect` (left/top/width/height) so `Text::Draw` keeps its centering math (`bb.width - text_bounds.width()`, etc.).
- Scalar overload delegates so `debug_overlay.cc:46-58` compiles unchanged (FR-012).

### 3. `Text::Draw` / `Button::Draw` (MODIFY text.cc/button.cc)

Replace the `#if __APPLE__ ... #else SkFont(nullptr,...) #endif` blocks (text.cc:43-49, button.cc:55-61) with:

```cpp
Font font{ s.font_family(), s.font_weight(), size };   // size = s.font_size()>0 ? : 16
Rect tb = canvas.MeasureText(text, font);
// ... same horizontal/vertical centering math using tb ...
canvas.DrawText(text, Point{tx,ty}, paint, font);
```

No `SkTypeface`/`SkFontMgr` in widgets anymore — zero Skia leakage (FR-008/009, render contract).

### 4. Skia FreeType + custom ports (MODIFY third_party/skia.BUILD, native_ui_deps.bzl)

- **Prerequisite**: add `freetype` output (http_archive, e.g. FreeType 2.13.2 + a `cc_library` BUILD with `FT2_BUILD_LIBRARY` and no `FT_CONFIG_OPTION_USE_PNG/...` to stay self-contained), add `@freetype//:freetype` dep to the `skia` cc_library.
- Un-exclude from `srcs` glob (skia.BUILD:22-24): `SkFontHost_FreeType*.cpp`, `SkTypeface_FreeType.cpp`, `SkFontMgr_custom*.cpp`. Keep `SkFontMgr_mac_ct` compiled (already un-excluded; Apple-guarded TU elsewhere).
- FreeType compiles on all platforms; on macOS the framework hands registered-file typefaces to CoreText and FreeType stays unused (dead-stripped) → no behavioral change on Apple (FR-012).
- Android: `__ANDROID__` picks the Android directory mgr automatically by `#if`.

### 5. Tests (NEW tests/font_manager_test.cc; MODIFY tests/widgets_test.cc, BUILD.bazel)

Host (macOS/Linux — same binary, metrics-based; no pixel-diff helper needed):
- Vendor `tests/assets/fonts/`: Roboto `Regular.ttf`, `Bold.ttf`, + one distinct display font (Apache-2.0, from pinned Skia `resources/fonts`) so width metrics are comparable and stable.
- `RegisterFont("demo",".../Roboto-Regular.ttf",400)` → `MeasureText("Hello",Font{"demo",400,24}).width() > 0` (**SC-001**; today 0).
- Register regular+bold → width(700) > width(400) for same glyphs shared without fallback (**SC-002**).
- Unknown family `Font{...,"no-such",400,24}` and missing-path register → no crash; with default registered it resolves to default metrics, otherwise no-op empty (FR-007/012). Missing/corrupt file → `RegisterFont` false + `last_error()` non-empty + default unchanged (FR-006).
- First register → `HasDefaultFont()` true, empty-family measure > 0 matching default (**SC-006/FR-013**). `SetDefaultFont` override re-points (FR-014); invalid family false.
- 10,000 `Resolve`(new families re-registered) loop → residency bounded (**SC-004/FR-011**).
- widgets_test: `Text(Content("Hi"), FontFamily("demo"))` registered render no-crash + non-empty measure; tag ≡ `ApplyStyle(Style)` render equal widths (FR-009). Existing snapshot/no-crash tests keep passing unchanged (FR-012/SC-005).

### 6. Android device demo (NEW examples/font_demo.cc + make target)

Mirror feature-011 device loop, minus codec: `RegisterFont("demo","/data/local/tmp/font/xxx.ttf")` (push asset + font via adb) → build `Container`/`Text(FontFamily("demo"))` → draw onto `Surface::Create(w,h)` → `Surface::Dump` → pull PNG; visually verify on device (FR-005/SC-001 on-device proof). Add `android-font-demo` make target reusing `android_build.sh`/`android_demo.sh` pattern (build `//examples:font_demo`, push font+bin, run, pull PNG).

## Complexity Tracking

> Constitution has no binding principles — no violations to justify.

## Implementation Order (task grouping)

0. **Prerequisite (blocking)**: vendor FreeType (`native_ui_deps.bzl` + `@freetype//:freetype` `cc_library`); un-exclude FreeType/custom ports in `third_party/skia.BUILD`; verify `bazel build //src/framework/render` on host is still green (macOS unchanged; FreeType links).
1. **FontManager module**: `render/font_manager.h/.cc` + render BUILD; per-platform mgr construction; registry/cache/default; `RegisterFont`/`SetDefaultFont`/`Resolve`/`last_error`.
2. **Canvas wiring**: `MeasureText` + `DrawText(Font)` overloads in canvas.h/.cc; scalar overload delegation.
3. **Widget wiring**: rewrite text.cc/button.cc to Font descriptors; remove `#if __APPLE__` blocks.
4. **Host verification**: vendor test fonts; `tests/font_manager_test.cc` + widgets_test additions; BUILD targets; run `bazel test //tests/...` host green; confirm existing text/no-crash tests unchanged (FR-012/SC-005).
5. **Android verification**: `examples/font_demo.cc` + BUILD + make target; `make android-build` + run on API 29+ device/emulator → pull PNG; manually confirm glyph rendering vs platform.
6. **Docs**: this plan + contracts stay in sync; AGENTS.md updated to point at 012 plan/contracts.
7. **Public API + constants (post-implementation polish)**: add `native_ui/font.h` public forwarding header (aggregated by `native_ui/render.h`) so external users consume `<native_ui/font.h>`; add `FontManager` constants — family-name sentinels (`kDefaultFontFamily`, `kSansSerifFamily`, `kSerifFamily`, `kMonospaceFamily`), common font sizes (`kFontSizeCaption…kFontSizeHero`), and weights (`kFontWeightRegular/Medium/Bold`); demo (`font_demo.cc`) + tests exercise them.