# Tasks: External Font Registration Interface

**Input**: Design documents from `specs/012-android-font-support/`

**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Included — the feature spec defines measurable host-test success criteria (SC-001/002/003/004/006) and plan.md §5 mandates `tests/font_manager_test.cc`.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

Repository root for Bazel targets: `native_ui/`. Feature paths:

- `native_ui/third_party/skia.BUILD`, `native_ui/third_party/freetype.BUILD`
- `native_ui/native_ui_deps.bzl`
- `native_ui/src/framework/render/`
- `native_ui/src/framework/widgets/`
- `native_ui/tests/`, `native_ui/examples/`, `native_ui/mk/`

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Vendor the FreeType dependency and test font assets required by every later phase.

- [X] T001 Add `freetype` http_archive (FreeType 2.13.2, sha256-pinned) to `native_ui/native_ui_deps.bzl`
- [X] T002 [P] Create `native_ui/third_party/freetype.BUILD` — `cc_library(name = "freetype")` compiling FreeType sources with `FT2_BUILD_LIBRARY`, freetype2 include path exported, no `FT_CONFIG_OPTION_USE_PNG/ZLIB/BZIP2` (self-contained)
- [X] T003 [P] Vendor test font assets under `native_ui/tests/assets/fonts/` (Apache-2.0, sourced from pinned Skia `resources/fonts`): `Roboto-Regular.ttf`, `Roboto-Bold.ttf`, and one distinct display font (e.g. `NotoSansDisplay-Regular.ttf`) — used for host metric assertions

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Enable a usable font rasterizer in the Skia build and install the core `FontManager` + `Canvas` font plumbing. MUST be complete before ANY user story.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete.

- [X] T004 Un-exclude Skia FreeType/custom ports in `native_ui/third_party/skia.BUILD` — remove `SkFontHost_FreeType*.cpp`, `SkTypeface_FreeType.cpp`, `SkFontMgr_custom*.cpp` from the `srcs` glob exclusions; add `@freetype//:freetype` to the `skia` cc_library deps with required include dirs and `SK_FREETYPE_MINIMUM_RUNTIME_VERSION` define if needed by the ports
- [X] T005 [P] Create `native_ui/src/framework/render/font_manager.h` — public `struct Font{std::string family; int weight; float size;}` and `class FontManager` (singleton `Default()`, `RegisterFont`, `SetDefaultFont`, `HasDefaultFont`, `last_error`, `Clear`, internal `Resolve`) per `contracts/font-manager.md`
- [X] T006 [P] Implement `native_ui/src/framework/render/font_manager.cc` — registry/cache/default members, per-platform `SkFontMgr` construction (`__APPLE__`→CoreText;`__ANDROID__`→CustomDirectory(`/system/fonts`);else→CustomDirectory(empty-dir)), `makeFromData(SkData::MakeFromFileName(path))` load, exact-weight `Resolve`, `|cache|≤|registry|+1` invariant
- [X] T007 Modify `native_ui/src/framework/render/BUILD.bazel` — ensure `font_manager.cc/.h` build into `//src/framework/render:render` (glob covers *.cc; confirm `@skia//:skia` dep present)
- [X] T008 [P] Add `Rect MeasureText(const std::string&, const Font&)` and `void DrawText(..., const Font&)` declarations to `native_ui/src/framework/render/canvas.h`; keep the existing scalar `float font_size` overload
- [X] T009 Implement the new overloads in `native_ui/src/framework/render/canvas.cc` — resolve typeface ONCE via `FontManager::Default().Resolve(family,weight)` used by both `measureText` and `drawString` (FR-008); scalar overload delegates to `Font{size}`
- [X] T010 Verify host build green: `bazel build //src/framework/render` from `native_ui/` — FreeType links, macOS behavior unchanged (FR-012/SC-005 gate)

**Checkpoint**: Foundation ready — user story implementation can now begin.

---

## Phase 3: User Story 1 - Register a font by file path and use it in text (Priority: P1) 🎯 MVP

**Goal**: A developer can set a font by external file path (`RegisterFont(family, path)`) and it renders on all platforms; the first registration becomes the implicit default.

**Independent Test**: `RegisterFont("demo", "tests/assets/fonts/Roboto-Regular.ttf", 400)` then `MeasureText("Hello", Font{"demo",400,24}).width() > 0` on host; on-device, `examples/font_demo.cc` produces a PNG of registered-font text.

### Tests for User Story 1

> **These tests are written FIRST and must FAIL before implementation (red-green).**

- [X] T011 [P] [US1] Add SC-001 test to `native_ui/tests/font_manager_test.cc`: `RegisterFont("demo", Roboto-Regular, 400)` → `MeasureText("Hello", Font{"demo",400,24}).width() > 0` (today 0)
- [X] T012 [P] [US1] Add SC-006/FR-013 test to `native_ui/tests/font_manager_test.cc`: first successful registration → `HasDefaultFont()` true; empty-family `MeasureText` > 0 and metrics match default family
- [X] T013 [P] [US1] Add FR-014/FR-010 tests to `native_ui/tests/font_manager_test.cc`: `SetDefaultFont` re-points default (invalid family → false + `last_error`, default unchanged); re-register same (family,weight) → refreshed metrics
- [X] T014 [P] [US1] Add widget tests to `native_ui/tests/widgets_test.cc`: `Text(Content("Hi"), FontFamily("demo"))` with registered font draws without crash + non-empty measure; tag render ≡ `ApplyStyle(Style{fontFamily("demo")})` render (FR-009)

### Implementation for User Story 1

- [X] T015 [US1] Rewrite `Text::Draw` in `native_ui/src/framework/widgets/text.cc` — build `Font{family, weight, font_size}` from `style()`, use `canvas.MeasureText`/`canvas.DrawText`, DELETE the `#if __APPLE__` SkFont blocks (keep centering math using returned bounds)
- [X] T016 [P] [US1] Rewrite `Button::Draw` in `native_ui/src/framework/widgets/button.cc` — same Font-descriptor pattern, remove `#if __APPLE__` block
- [X] T017 [US1] Add `font_manager_test` cc_test target to `native_ui/tests/BUILD.bazel` (deps: `//src/framework/render` + gtest; `data = ["assets/fonts/Roboto-Regular.ttf", "assets/fonts/Roboto-Bold.ttf", "assets/fonts/NotoSansDisplay-Regular.ttf"]`); ensure tests call `FontManager::Default().Clear()` for isolation

**Checkpoint**: At this point, User Story 1 is fully functional and independently testable (MVP).

---

## Phase 4: User Story 2 - Register different weight variants of a family (Priority: P2)

**Goal**: `RegisterFont(family, path, weight)` per weight; `FontWeight(...)` picks the exact or nearest registered variant.

**Independent Test**: Register regular+bold → `MeasureText` width(700) > width(400) of the same string using the same family; nearest-weight selection works for gaps.

### Tests for User Story 2

- [X] T018 [P] [US2] Add SC-002 test to `native_ui/tests/font_manager_test.cc`: register `demo`@400(Roboto-Regular) + `demo`@700(Roboto-Bold) → `MeasureText("Hello", Font{"demo",700,24}).width() > MeasureText("Hello", Font{"demo",400,24}).width()` in the expected direction
- [X] T019 [P] [US2] Add FR-004 tests to `native_ui/tests/font_manager_test.cc`: register only 400 & 900, request 500 → 400 selected; register single-variant family, request any weight → that file used, no crash

### Implementation for User Story 2

- [X] T020 [US2] Implement nearest-weight resolution in `Resolve` in `native_ui/src/framework/render/font_manager.cc` — exact match first; else min `|Δweight|` (tie → lower); single variant if family has one; `weight<=0` normalized to 400

**Checkpoint**: At this point, User Stories 1 AND 2 both work independently.

---

## Phase 5: User Story 3 - Graceful handling of unknown families and bad font files (Priority: P3)

**Goal**: Unknown families and corrupt/missing files fall back to the default font with a clear observable error, never crashing; platform-default preserved when nothing is registered.

**Independent Test**: Register a missing path and render `FontFamily("never_registered")` → no crash, falls back to default font.

### Tests for User Story 3

- [X] T021 [P] [US3] Add SC-003/FR-006 test to `native_ui/tests/font_manager_test.cc`: missing/corrupt path → `RegisterFont` returns false, `last_error()` non-empty, default entry unchanged; unknown family + default registered → resolves to default metrics (FR-007)
- [X] T022 [P] [US3] Add FR-012/SC-005 regression test in `native_ui/tests/font_manager_test.cc`: no registration at all → `MeasureText` with empty family renders no-op empty, no crash (platform-default path unchanged)

### Implementation for User Story 3

- [X] T023 [US3] Harden error handling in `native_ui/src/framework/render/font_manager.cc` — load failure sets `last_error_`, never crashes, family falls back to default; `SetDefaultFont` on unknown family → false + error; `Resolve` never returns null (default or no-op empty typeface)

**Checkpoint**: All user stories now independently functional.

---

## Phase 6: Android Device Demo

**Purpose**: On-device proof of registered-font rendering (FR-005), reusing the feature-011 device tooling.

- [X] T024 [P] Create `native_ui/examples/font_demo.cc` — `RegisterFont("demo", <pushed path>)` → build `Text(Content(...), FontFamily("demo"))` in a container → draw onto `Surface::Create(w,h)` → `Surface::Dump` PNG (mirrors `external_image_demo.cc` loop, no codec)
- [X] T025 [P] Add `font_demo` cc_binary to `native_ui/examples/BUILD.bazel` (deps: `//src/framework/public:native_ui`, surface; `data`: a font asset)
- [X] T026 Add `android-font-demo` make target in `native_ui/mk/` reusing `android_build.sh`/`android_demo.sh` pattern — build `//examples:font_demo`, push binary+font to device, run, pull PNG

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Full-suite validation and documentation consistency across all stories.

- [X] T027 [P] Run full host suite from `native_ui/`: `bazel test //tests/...` — all green; existing text/no-crash/snapshot tests unchanged (FR-012/SC-005)
- [X] T028 [P] Run `bazel build //examples:font_demo` for `android_arm64` (`--config android_arm64`) to confirm the device demo cross-compiles
- [X] T029 [P] Verify `AGENTS.md` SPECKIT block points at `specs/012-android-font-support/plan.md` and `contracts/font-manager.md`; keep `quickstart.md` examples consistent with the shipped API
- [X] T030 Update `specs/012-android-font-support/quickstart.md` if signature/behavior details changed during implementation

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion — BLOCKS all user stories
- **User Story 1 (Phase 3)**: Depends on Foundational — no dependency on other stories (MVP)
- **User Story 2 (Phase 4)**: Depends on Foundational + US1 (weight resolution on top of registry) — independently testable
- **User Story 3 (Phase 5)**: Depends on Foundational + US1 default mechanism — independently testable
- **Android Demo (Phase 6)**: Depends on US1 (rendering path)
- **Polish (Phase 7)**: Depends on all desired user stories

### Within Each User Story

- Tests FIRST (must fail before implementation), then implementation
- Foundational plumbing (T004–T010) before story code
- Story complete before moving to next priority

### Parallel Opportunities

- All Setup tasks (T001–T003) run in parallel
- Foundational tasks T005/T008 are [P] (separate headers); T004, T006, T009, T010 are sequential build blocks
- All tests for a story run in parallel ([P])
- T015/T016 (text.cc vs button.cc) run in parallel
- Phase 6 tasks T024/T025 run in parallel
- Phase 7 tasks T027–T030 run in parallel

---

## Parallel Example: User Story 1

```bash
# Launch tests for US1 together:
Task: "Add SC-001 test to native_ui/tests/font_manager_test.cc (T011)"
Task: "Add SC-006/FR-013 test to native_ui/tests/font_manager_test.cc (T012)"
Task: "Add FR-014/FR-010 tests to native_ui/tests/font_manager_test.cc (T013)"
Task: "Add widget tests to native_ui/tests/widgets_test.cc (T014)"

# Then implement:
Task: "Rewrite Text::Draw in native_ui/src/framework/widgets/text.cc (T015)"
Task: "Rewrite Button::Draw in native_ui/src/framework/widgets/button.cc (T016)"
Task: "Add font_manager_test target to native_ui/tests/BUILD.bazel (T017)"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (FreeType + font assets)
2. Complete Phase 2: Foundational (Skia ports + FontManager + Canvas)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: `bazel test //tests:font_manager_test //tests:widgets_test` on host
5. Deploy/demo if ready

### Incremental Delivery

1. Setup + Foundational → foundation ready (existing tests still green)
2. US1 → register + render + implicit default → test independently → MVP
3. US2 → weight variants → test independently
4. US3 → fallback/error-hardening → test independently
5. Android demo → on-device PNG proof

### Parallel Team Strategy

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: US1 (T011–T017)
   - Developer B: US2 (T018–T020, after US1 registry exists)
   - Developer C: US3 (T021–T023, after US1 default exists)
3. Then Android demo (T024–T026) and Polish (T027–T030)

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to a user story for traceability
- Each user story is independently completable and testable
- Verify tests fail before implementing (red-green)
- Commit after each task or logical group
- Avoid: vague tasks, same-file conflicts, cross-story dependencies that break independence