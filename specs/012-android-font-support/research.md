# Research: External Font Registration Interface

**Feature**: `012-android-font-support` | **Phase 0 output of `/speckit.plan`**

## Questions resolved in this phase

The feature was re-scoped through clarification: the deliverable is **a framework interface by which the developer sets a font file path externally (register a named family), effective on all platforms**. No `[NEEDS CLARIFICATION]` markers remain in the spec. All technical unknowns below were resolved by inspecting the codebase and pinned Skia source; where the pinned Skia tarball was not present on the machine, findings come from the vendored-file investigation and the pinned commit `fdbe1458`.

### Q1. Why do text properties currently do nothing, exactly?

**Decision**: Family/weight are collected into `style_` but never read at draw time; and on Android/Linux no rasterizer is compiled, so even a typeface attempt yields an empty typeface.

**Rationale** (evidence):
- `Canvas::DrawText(text, pos, paint, font_size)` carries only a scalar size (`render/canvas.h:28-29`). `text.cc:43-49` and `button.cc:55-61` build a measuring `SkFont` that never reads `style().font_family()` / `style().font_weight()`, then call `DrawText(text, pt, paint, size)` (`text.cc:67`, `button.cc:67`) — family/weight die at the widget→canvas boundary.
- `third_party/skia.BUILD:23-31` excludes every non-CoreText font port (`SkFontMgr_android*`, `SkFontMgr_custom*`, `SkFontHost_FreeType*`, …). `SkFontMgr_mac_ct.cpp` is guarded to `SK_BUILD_FOR_MAC|IOS` (empty TU elsewhere). Therefore on Android/Linux **no `SkFontMgr` exists** and `SkFont(nullptr, …)` inside that Skia revision substitutes `SkTypeface::MakeEmpty()` (`src/core/SkFont.cpp:56-60`) → **text is invisible today**.
- `style.cc` stores set-flags per property (`data_.font_family_`/`_set_`), getters (`style.h:95-102`); there is no public `is_set()` accessor, which is why `text.cc:40` uses the `> 0.0f` heuristic for size.

**Alternatives considered**: none (this is a factual code-state finding, not a choice).

### Q2. Which font manager / rasterizer do we build on?

**Decision**: Compile Skia's **FreeType-based port set** (`SkFontHost_FreeType*`, `SkTypeface_FreeType`, `SkFontMgr_custom*`) plus a `freetype` source dependency into the Skia build, and let the framework construct the platform `SkFontMgr` explicitly (mirroring the existing `SkFontMgr_New_CoreText` pattern, `canvas.cc:146`) instead of relying on `SkFontMgr::Factory` wiring.

**Rationale**:
- The spec's core is file-path registration (`makeFromFile`/`makeFromData`). Every port that supports it — CoreText (macOS), Android/system, custom-directory (FreeType) — is reachable, and FreeType is the cross-platform rasterizer that makes registered fonts *render real glyphs* on Android/Linux. Without it, registered typefaces remain empty (Q1 finding).
- Explicit per-platform construction preserves the current CoreText default on Apple (FR-012) without changing `SkFontMgr::Factory`/macro wiring in the pinned Skia, and follows the codebase precedent (`canvas.cc:145-151`).
- macOS keeps `SkFontMgr_New_CoreText` as the default manager; registration on macOS goes through the same manager (`makeFromData`), so the interface is truly all-platform. FreeType sources compile on the host build but remain unused on Apple; dead-stripped, negligible payload.

**Alternatives considered**:
- `SkFontMgr_android` (system `fonts.xml` semantics) — not needed for the file-path interface, requires `SkFontMgr_android_parser` + a custom finder compiled; deferred. Unknown-family resolution on Android uses the custom-directory manager instead (see Q3). System-family-name aliasing (`sans-serif`) documented out of scope in spec Assumptions.
- Rely on `SkFontMgr::Factory()` being enabled by config macros — fragile in a raw-`glob` cc build; explicit construction is the pattern the repo already uses.

### Q3. What is the default/fallback font on each platform?

**Decision**:
- macOS: CoreText manager default typeface — unchanged from today (FR-012).
- Android: `SkFontMgr_New_Custom_Directory("/system/fonts")` — real device glyphs for the fallback/default path, and gives registered-file typefaces a FreeType backend on-device.
- Linux/host: `SkFontMgr_New_Custom_Directory(default_dir)` with a no-op dir default → default typeface behaves the same as today (empty); the *registered* path always renders real glyphs. Host tests therefore verify the registered path with committed font assets and keep default-behavior tests to "no crash, no regression".

**Rationale**: keeps every platform's existing default behavior (FR-012) while guaranteeing the new interface renders on all three (FR-005). Unknown family → platform default manager; corrupt/missing file → default manager (FR-006/007).

**Alternatives considered**: vendoring a bundled default font into the framework — rejected (spec Assumptions keep framework font-free; branch/scope).

### Q4. How do we satisfy FR-008 (measure with the same typeface that draws)?

**Decision**: Move both measurement and drawing into `Canvas` through the new `Font` descriptor + `FontManager`. `Text::Draw`/`Button::Draw` stop constructing `SkFont`; they build a value-typed `Font{family, weight, size}` from `style()` and call `canvas.MeasureText(text, font)` and `canvas.DrawText(text, pos, paint, font)`. `Canvas` resolves the typeface once through `FontManager` (cached) and uses that same `sk_sp<SkTypeface>` for both `measureText` and `drawString`.

**Rationale**: single resolution site inside the Skia-encapsulated `render` layer, keeps widgets Skia-free (respects the "Skia fully encapsulated" rule in `surface.h:38-39`), and makes tags ≡ applied-`Style` trivially true because both flow through `style()` (FR-009).

**Alternatives considered**: exposing `SkTypeface` to widgets — rejected (leaks Skia). Returning an opaque `FontHandle`/id from a widget-accessible resolver — unnecessary indirection; the descriptor+Canvas approach is simpler and satisfies FR-008 directly.

**Design note**: keep the existing scalar `DrawText(text, pos, paint, font_size)` overload delegating to a default `Font` so `debug_overlay.cc:46-58` keeps compiling unchanged (FR-012).

### Q5. How do we keep memory bounded (FR-011) and refresh on re-registration (FR-010)?

**Decision**: `FontManager` holds a `(family, weight) → TypefaceResource{cached sk_sp<SkTypeface>, held sk_sp<SkData>}` cache. Unbounded growth is prevented because the lookup key set is exactly the set of registered `(family, weight)` entries plus any 1:1 aliases; repeated resolutions hit the cache. Registering the same family with a new path evicts that family's cache entries so the next resolve reloads (FR-010).

**Rationale**: cache key cardinality is bounded by registrations (an app registers a handful of fonts), so 10k resolutions cannot grow memory (SC-003/FR-011); `SkData` is retained so the typeface's backing bytes survive (FreeType requirements).

**Alternatives considered**: LRU/eviction policy — unnecessary; registration set is the natural bound. Per-call file re-read — rejected (FR-011/perf).

### Q6. How do we verify all-platform behavior (FR-005, SC-001/002/003)?

**Decision**: two layers:
1. **Host unit tests** (`tests/font_manager_test.cc` + additions to `tests/widgets_test.cc`) register committed font assets under `tests/assets/fonts/` (Roboto Regular/Bold + a distinct display font, Apache-2.0, vendored from the pinned Skia `resources/fonts`), then assert measurable outcomes: non-empty measured bounds for a registered family (SC-001), Regular-vs-Bold metric differences for the same family (SC-002), fallback/crash-free behavior for unknown family and missing file (SC-003), and a `10,000`-resolution bounded-memory loop (SC-004). No pixel-diff machinery is required on host.
2. **Android device demo** (`examples/font_demo.cc`) registers a pushed font path, draws `Text` to a raster `Surface`, and writes `Surface::Dump` PNG; reused via a `android-font-demo` make target so the output is pulled off-device and viewed (mirrors feature 011's device loop).

**Rationale**: the feature's real acceptance is measurable through text metrics on host (no GPU/screen needed) while the on-device demo proves glyph rasterization on Android with an actual font file — the combination covers SC-001/002/003 without adding a host pixel-diff helper.

**Alternatives considered**: adding a host `Surface` pixel-readback helper — deferred (not required by spec; metrics are the measurable contract).

### Q7. How is the default font designated?

**Decision**: The **first successfully registered font** becomes the framework default (`FontManager::DefaultFont()` → first-family resolver), and an explicit `SetDefaultFont(family)` overrides the first-registered choice (spec FR-013/FR-014). Resolution precedence for a `Font{family, weight, size}` lookup:
1. family == "" or unset → default font (if registered) else platform default.
2. family registered → matching/nearest-weight variant.
3. family unknown, or the registered path failed to load → default font (if registered) else platform default.

The default-font pointer is a `(family, weight)` reference into the same registry/cache, so re-registering the default family to a new path refreshes it (FR-010); explicit change via `SetDefaultFont` simply repoints the default reference (FR-014) — no second copy of a typeface.

**Rationale**: "empty family renders the first registered font" is the developer-facing value the user asked for; routing unknown/corrupt fallbacks through the same default gives one consistent fallback rule (clarification Q2/A) and keeps SC-003/SC-006 measurable on host. Default precedence is trivially covered by the existing cache-bounded guarantee (SC-004).

**Alternatives considered**:
- System/`Style::SetDefault`-style global theme default — out of scope (spec Assumptions: font-free framework default; the default only exists once a registration is made).
- Always platform default for unknown families even when a default font exists — rejected by clarification Q2/A (unified fallback).

## Design snapshot (settled by research)

- New `render/font.h` (value type `Font{family, weight, size}`) + `render/font_manager.h/.cc` (registry+platform manager+cache, `FontManager::Default()` singleton).
- `Canvas` gains `MeasureText(const Font&) → Rect` and a `DrawText(..., const Font&)` overload; scalar overload preserved for `debug_overlay`.
- `Text::Draw`/`Button::Draw` switch to descriptors (remove `#if __APPLE__` blocks).
- Skia build: add `freetype` http_archive + re-include excluded FreeType/custom ports (`third_party/skia.BUILD`).
- Tests: `tests/font_manager_test.cc` (+ widget tests) with vendored font assets; Android example `examples/font_demo.cc` + make target.