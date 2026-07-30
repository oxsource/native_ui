---

description: "Task list for Phase 10: SVG Image Support"

---

# Tasks: SVG Image Support

**Input**: Design documents from `specs/010-svg-image-support/`

**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

All paths are relative to `native_ui/` under the repository root.

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Integrate nanosvg third-party library and create build infrastructure

- [x] T001 Create `third_party/nanosvg/BUILD.bazel` — `cc_library(name = "nanosvg", hdrs = ["src/nanosvg.h", "src/nanosvgrast.h"], includes = ["src"], visibility = ["//src/framework:__subpackages__"])`
- [x] T002 [P] Add `_nanosvg()` http_archive rule to `native_ui_deps.bzl` — URL + sha256 for nanosvg tarball
- [x] T003 Remove locally committed nanosvg headers (now fetched via http_archive Bazel rule)

---

## Phase 2: Foundation — SVG Rasterization + Image::FromFile Extension (Priority: P1) 🎯 MVP

**Purpose**: Extend `Image::FromFile()` to detect `.svg` files and rasterize via nanosvg. Add `Image::FromSkImage()` internal factory. This is the core SVG support that all consumers use.

**Independent Test**: Call `Image::FromFile("assets/photo/superdog.svg")` — verify non-null Image with correct dimensions. Call with nonexistent path — verify null.

- [ ] T004 Add `FromSkImage(sk_sp<SkImage>)` private/internal factory to `native_ui/src/framework/render/image.h` — creates an Image wrapping an existing SkImage (used by nanosvg path)
- [ ] T005 Implement SVG detection and rasterization in `native_ui/src/framework/render/image.cc` — in `Image::FromFile`, check `.svg` extension; if SVG, call nanosvg parse + rasterize, wrap result in Image via FromSkImage
- [ ] T006 Add `//third_party/nanosvg` dep to `native_ui/src/framework/render/BUILD.bazel`

**Checkpoint**: SVG files load and rasterize via `Image::FromFile()` — returns valid Image with pixels.

---

## Phase 3: User Story 1 - Glide Async SVG Loading (Priority: P1)

**Goal**: Glide::Load handles SVG files transparently on worker threads (just like PNG). ImageWidget with ImageURI loads SVG asynchronously.

**Independent Test**: Set up Glide with DefaultGlide, call `Glide::Load("assets/photo/superdog.svg", callback)`, verify callback delivers a valid Image on main thread.

- [ ] T007 [P] [US1] Verify Glide worker thread SVG loading — Glide calls `Image::FromFile()` on worker thread which now handles `.svg` via nanosvg (no Glide changes needed, integration is automatic)

**Checkpoint**: Glide loads SVG files asynchronously without blocking main thread.

---

## Phase 4: User Story 2 - Image Gallery Example (Priority: P2) 🎯 FINAL

**Goal**: `examples/image_gallery.cc` with 5 ImageWidget+Text cards in a Container(Column) — 2 SVG (kCenter, kCenterInside) + 3 PNG (kCenterCrop, kCenterInside, kFillXY). All loading via Glide async. Output single `/tmp/image_gallery.png`.

**Independent Test**: `bazel run //examples:image_gallery` produces `/tmp/image_gallery.png` showing 5 distinct cards with visible image content and Text labels.

- [ ] T008 [P] [US2] Create `examples/image_gallery.cc` — initialize Glide with DefaultGlide, build Container(Column) with 5 cards, each card is Container(Column) with ImageWidget(ImageURI, ScaleType) + Text(Content) label
- [ ] T009 [US2] Add `image_gallery` cc_binary target to `examples/BUILD.bazel` with deps on `//src/framework/widgets`, `//src/framework/render`, `//src/framework/utils`, `//src/framework/surface`, `//src/framework/public:native_ui`
- [ ] T010 [US2] Ensure `assets/photo/` paths resolve at runtime — use Bazel `data` attribute or absolute path fallback for police.png and superdog.svg

**Checkpoint**: `bazel run //examples:image_gallery` produces `/tmp/image_gallery.png` with 5 visible cards.

---

## Phase 5: Verification

**Purpose**: Verify that the SVG loading pipeline and image gallery example work correctly

- [ ] T011 Verify `Image::FromFile` returns non-null for `superdog.svg` — write a quick smoke test or manually check via example
- [ ] T012 Verify `bazel build //...` succeeds with the new nanosvg dep
- [ ] T013 Run image gallery and verify `/tmp/image_gallery.png` exists and has expected card layout

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — nanosvg headers must be fetched first
- **Foundation (Phase 2)**: Depends on Setup — BLOCKS all SVG functionality
- **US1 Glide SVG (Phase 3)**: Depends on Phase 2 — automatic, no code changes needed
- **US2 Example (Phase 4)**: Depends on Phase 2 + Phase 3
- **Verification (Phase 5)**: Depends on all phases

### User Story Dependencies

- **US1 (SVG Loading) P1**: Depends on nanosvg integration in Image::FromFile
- **US2 (Image Gallery) P2**: Depends on SVG loading + Glide

### Parallel Opportunities

- T001+T002 (Setup) — [P]
- T004+T005+T006 (Foundation) — sequential
- T007 (US1) — no new code (automatic via Glide)
- T008+T009+T010 (US2) — T008+T009 are [P]
- US1 and US2 are sequential (US2 depends on US1 functioning)

---

## Implementation Strategy

### MVP (SVG Loading Only)

1. Phase 1: Fetch nanosvg headers + BUILD
2. Phase 2: Extend Image::FromFile for SVG
3. **Validate**: `Image::FromFile("superdog.svg")` returns valid Image

### Incremental Delivery

1. SVG loads via Image::FromFile
2. SVG loads via Glide async (automatic)
3. Image Gallery example with 5 cards
4. Visual verification via output PNG

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- nanosvg is fetched via http_archive — no local files in third_party/nanosvg/ besides BUILD.bazel
- Include as `#include "src/nanosvg.h"` — the `strip_prefix` places headers under `src/`
- `Image::FromSkImage(sk_sp<SkImage>)` is an internal helper — not part of the public API
- SVG rasterization happens at load time at natural SVG size; ScaleType at render time handles widget bounds
- Glide worker thread calls `Image::FromFile` which now handles both PNG and SVG
- The image gallery example assets path may need adjustment — assets/photo/ is relative to the workspace root, not the binary output directory
- `assets/photo/` can be added as Bazel `data` dep or the example can construct the absolute path from `TEST_SRCDIR` or `BAZEL_WORKSPACE` env
