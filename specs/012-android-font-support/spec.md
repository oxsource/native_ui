# Feature Specification: External Font Registration Interface (All Platforms)

**Feature Branch**: `012-android-font-support`

**Created**: 2026-08-20

**Status**: Draft

**Input**: User description: "在安卓平台支持设置字体" + clarification: "需要你提供接口，我从外部设置字体路径这样的" → Provide a framework interface so the developer sets font file paths externally; the interface works on all platforms.

## Clarifications

### Session 2026-08-20

- Q: 默认字体如何选择？ → A: 首个注册字体作为隐式默认，同时提供显式 `SetDefaultFont(family)` 覆盖。
- Q: 默认字体的回退优先级？ → A: 存在默认字体时，空/未设置、未知族名、损坏文件三场景一律回退到默认字体；仅当无任何注册字体时才回退到平台默认字体。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Register a font by file path and use it in text (Priority: P1)

A developer using this framework wants the app texts to use a specific font file (`.ttf`/`.otf`) owned by the app, obtained from anywhere at runtime. At startup they call the framework's font registration interface, passing a family name and the font file's path. Later they label any `Text` (or `Button`) widget with that family name via the existing `FontFamily(...)`. When the widget draws, text renders with the registered font. Today such file-based font configuration is impossible: the framework has no interface to accept a font path, and family/weight properties are collected but ignored at draw time.

**Why this priority**: This is the entire value of the feature — a developer-facing interface to set a font file path from outside, taking effect in real rendering. Without it nothing else in the feature exists.

**Independent Test**: An app calls `RegisterFont("appfont", "/data/fonts/app.ttf")` and renders `Text(Content("Hello"), FontFamily("appfont"))`. The drawn glyphs use the registered file (host test: measured text bounds are non-empty for the registered family; on-device: rendered output matches a reference render of that font file).

**Acceptance Scenarios**:

1. **Given** a font file registered under a family name with a valid path, **When** a `Text` widget references that family name, **Then** the text renders with the registered font (measured bounds non-empty and consistent with that font's metrics; previously such text rendered empty/invisible).
2. **Given** the same registration API, **When** it is called on each supported platform (Android, macOS, Linux), **Then** the same code path registers and the family renders identically in behavior (all-platform parity).
3. **Given** a family registered with a valid path, **When** the family property is set through constructor tags **and** separately through an applied `Style`, **Then** both paths render the same font.
4. **Given** a font registered as the first registration, **When** a widget renders with no family specified, **Then** the text uses the first-registered font as the framework default.

---

### User Story 2 - Register different weight variants of a family (Priority: P2)

A developer has ordinary and bold files of the same typeface (e.g., `ttf regular` and `ttf bold`). They register both files under one family name with different weights. A `Text` widget using `FontWeight(...)` gets the matching variant; when a weight without an exact file is requested, the nearest registered variant is used.

**Why this priority**: Weight is a core text attribute and the existing `FontWeight` property is currently dead. It builds directly on the registration interface (Story 1) and makes the interface complete for real use; it ships and tests independently after Story 1.

**Independent Test**: Register regular and bold files for one family; render the same string at weight 400 and 700; the two renders use different files (host test: measured glyph metrics differ in the expected direction; on-device: visual difference visible in output).

**Acceptance Scenarios**:

1. **Given** a family with regular and bold files registered, **When** weight 700 is requested, **Then** the bold file is selected (metrics/rendering differ from the regular weight).
2. **Given** a family with only weight-400 and weight-900 files registered, **When** weight 500 is requested, **Then** the nearest variant (400) is used without error.
3. **Given** a family registered with a single file (no weight variants), **When** any weight is requested, **Then** the single file is used rather than crashing.

---

### User Story 3 - Graceful handling of unknown families and bad font files (Priority: P3)

A developer references a family name that was never registered, or registers a font path that is missing, unreadable, or corrupt. The framework does not crash, does not corrupt the rest of the UI, and reports a clear, observable error; the affected text falls back to the default font (the first registered font, unless explicitly overridden) so the app stays usable.

**Why this priority**: Robustness of a developer-facing API matters, but only after the registration path (Stories 1–2) demonstrably works. It is the final hardening slice and tests independently after the first two.

**Independent Test**: Register a bogus path (e.g., a nonexistent file) and render `FontFamily("never_registered")`; the app keeps running and text is drawn with the default font rather than disappearing or crashing.

**Acceptance Scenarios**:

1. **Given** an unregistered family name, **When** a widget references it, **Then** the framework falls back to the default font and does not crash.
2. **Given** a registered path that is missing or corrupt, **When** the family is used, **Then** the framework surfaces a clear error (observable to the developer) and falls back to the default font.
3. **Given** an empty or unset family name, **When** a widget renders, **Then** the default font is used and existing behavior is unchanged (no regression).

---

### Edge Cases

- What happens when the same family name is registered twice (same path / different path)? (Latest registration wins deterministically; cached entries are refreshed.)
- What happens when the first-registered default is later re-registered to a new path? (Default designation persists on the family; the resolved render uses the refreshed path.)
- What happens when the default font is explicitly changed via the default-font interface? (The newly designated family becomes the default; all fallback renders use it.)
- What happens when a font file path has an unsupported format or zero-length file? (Clear error; fallback to default font.)
- What happens when many different families/weights are used at once? (Resolved fonts are reused/cached; memory stays bounded.)
- What happens during live text updates or frequent re-renders? (Resolution is cached; no per-frame reload of font files.)
- What happens when a string contains glyphs the registered font lacks (e.g., CJK with a Latin-only font)? (Best-effort glyph fallback; renders as much as the platform supports; no blank crash.)
- What happens when the default platform font is used and the platform has no system font manager compiled? (Kept from regressing; for the default-only case the renderer must not crash — see Assumptions.)
- What happens on macOS where a CoreText default already works? (CoreText default path is preserved; registration-by-path supersedes it for registered families.)

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The framework MUST provide a developer-facing interface to register a font by supplying a family name and a font file path (the external-configuration entry point that drives this feature).
- **FR-002**: The registration interface MUST accept a weight (100–900) so multiple files of one family can be registered as variants; weight defaults sensibly when omitted.
- **FR-003**: A `Text` or `Button` widget referencing a registered family name MUST render using the registered font (this is the observable effect of FR-001).
- **FR-004**: When a weight variant with no exact registration is requested, the framework MUST select the nearest registered variant of that family; if no variant exists, use the single registration.
- **FR-005**: The registration interface MUST work identically on all supported platforms (Android, macOS, Linux) for the same application code.
- **FR-006**: Registering a missing, unreadable, or corrupt font file MUST produce a clear, observable error and MUST NOT crash; the affected family falls back to the framework default font.
- **FR-007**: Referencing an unregistered or unknown family name MUST fall back to the framework default font without crashing.
- **FR-008**: Text measurement and text drawing MUST use the same resolved font, so alignment/centering matches the drawn glyphs.
- **FR-009**: Font properties set via constructor tags and via the applied-style mechanism MUST both take effect identically.
- **FR-010**: Re-registering an existing family MUST refresh the cached resolution (new path takes effect immediately).
- **FR-011**: The system MUST keep memory bounded when fonts are resolved repeatedly; 10,000 successive resolutions MUST NOT cause unbounded memory growth.
- **FR-012**: The framework MUST preserve existing text rendering behavior where no registration or family is set (no regression to the default platform text path).
- **FR-013**: The framework MUST designate the first successfully registered font as the default font; text whose family is empty, unset, unknown, or fell back from a bad file MUST render with the default font when one exists.
- **FR-014**: The framework MUST provide an explicit `SetDefaultFont`-style interface to designate any registered family as the default font, overriding the first-registered choice.

### Key Entities *(include if feature involves data)*

- **Font family name**: The developer-chosen identifier under which font files are registered and referenced by `FontFamily(...)`. External configuration point of the feature.
- **Registered font entry**: A (family name, weight, font file path) record. Multiple weights may share a family name. Registers/unregisters over a feature's lifetime.
- **Resolved typeface**: The concrete font resource selected from a family+weight lookup (registered entry or platform default). Produced lazily, cached, and reused across widgets and frames.
- **Font resolution cache**: The map from (family, weight) to a resolved typeface, bounded and refreshed on re-registration.
- **Default font**: The font used when a widget does not specify a family (empty/unset) or when resolution falls back. Designated implicitly as the first successfully registered font, and overridable explicitly via default-font interface (FR-013/FR-014).

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Registering a font by path and referencing it renders text on all three supported platforms; 100% of host test cases produce non-empty measurements for a registered family (previously empty).
- **SC-002**: In 100% of compared test cases, a bold (700) variant render produces different glyph metrics from the regular (400) render of the same family, in the expected direction.
- **SC-003**: 100% of unregistered-family and corrupt/missing-file test cases fall back to the default font without crashing. When a default font exists, fallback uses that font (not the platform default) in 100% of cases.
- **SC-004**: Application memory stays bounded across 10,000 successive font resolutions (no measurable unbounded growth attributable to the feature).
- **SC-005**: No regressions: the existing default text-rendering tests on macOS/Linux continue to pass unchanged, and the CoreText default path on Apple platforms is preserved.
- **SC-006**: With a designated default font, 100% of host test cases render text with an empty/unset family using the default font (non-empty measurements, previously empty).

## Assumptions

- The core deliverable is the developer-facing registration interface (family name + font file path); resolving Android system font family names (e.g., "sans-serif") beyond registered families is out of scope for this feature and handled through the fallback-to-default path.
- Font files are supplied by the developer from outside the framework (any path reachable by the process at registration time). Bundling default fonts with the framework is out of scope; tests use a small font asset committed under the test directory.
- The existing `FontFamily`, `FontWeight`, and `FontSize` style properties are the reference mechanism; only the file-backed family registration is new.
- Weight is expressed as 100–900 (existing contract). A registered entry with no weight uses the conventional default (400).
- Registration happens before first draw (startup); live re-registration mid-render is supported (cache refresh, FR-010) but not required to be thread-safe against concurrent registration.
- Unknown-family fallback resolves to the framework default font when one exists (FR-013), and otherwise to the platform default font. On platforms that have no usable default rasterizer (see research), the fallback returns a no-op empty render only if the platform previously did the same — the registration path itself must always render real glyphs on all platforms.
- Where a glyph is missing from the chosen font, best-effort platform glyph fallback applies; comprehensive script fallback tables are out of scope.