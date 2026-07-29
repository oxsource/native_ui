---

description: "Task list for Skia Render Wrapper & Surface"

---

# Tasks: Skia Render Wrapper & Surface

**Input**: Design documents from `specs/005-skia-render-surface/`

## Phase 1: BUILD Infrastructure

- [x] T001 Update `src/framework/render/BUILD.bazel` — cc_library with hdrs, srcs, includes, dep on `@skia//:skia`, `//src/framework/core`, and `//src/framework/surface` (Canvas uses Surface&)
- [x] T002 Update `src/framework/surface/BUILD.bazel` — cc_library with hdrs, srcs, includes, dep on `@skia//:skia` and `//src/framework/core`
- [x] T003 Add test targets to `tests/BUILD.bazel` — `render_test` (dep on `//src/framework/render`), `surface_test` (dep on `//src/framework/surface`)

## Phase 2: Paint & Path (header-only wrappers)

- [ ] T004 Create `src/framework/render/paint.h` — `Paint` class: `SetColor`, `SetAntiAlias`, `SetStrokeWidth`, `SetStyle`, `SetAlpha`, chainable, wraps SkPaint
- [ ] T005 Create `src/framework/render/path.h` and `path.cc` — `Path` class: `MoveTo`, `LineTo`, `CubicTo`, `Close`, wraps SkPath

## Phase 3: HardwareBuffer & Surface

- [ ] T006 [P] Create `src/framework/surface/hardware_buffer.h` — `HardwareBuffer` header-only, platform `#ifdef` dispatch (IOSurfaceRef / dma-buf fd), `IsValid()`
- [ ] T007 [P] Create `src/framework/surface/surface.h` and `surface.cc` — `Surface` class: `Create(w,h)`, `CreateFromBuffer(HardwareBuffer)`, `Flush()`, wraps `sk_sp<SkSurface>`
- [ ] T008 Create `src/framework/surface/surface_factory.h` and `surface_factory.cc` — `SurfaceFactory` platform dispatch via `#ifdef`

## Phase 4: Image (lazy decode)

- [ ] T009 Create `src/framework/render/image.h` and `image.cc` — `Image` class: `FromEncoded`, `FromFile`, `FromBuffer`, lazy decode on draw, width/height accessors

## Phase 5: Canvas (RAII drawing context)

- [ ] T010 Create `src/framework/render/canvas.h` and `canvas.cc` — `Canvas` class: constructor saves SkCanvas state, destructor restores, `DrawRect`, `DrawText`, `DrawPath`, `DrawImage`, `Save`, `Restore`, `ClipRect`, `Translate`

## Phase 6: Unit Tests

- [ ] T011 Create `tests/render_test.cc` — test: Canvas save/restore state correctness (8+ levels)
- [ ] T012 Add test: Paint chainable builder — SetColor + SetAntiAlias + SetStrokeWidth returns correct values
- [ ] T013 Add test: Path construction — MoveTo/LineTo/CubicTo/Close produces correct point count
- [ ] T014 Add test: Canvas DrawRect with pixel readback — verify correct pixel color in rect area
- [ ] T015 Add test: Canvas DrawRect zero/negative dimensions — no crash, graceful handling
- [ ] T016 Add test: Image::FromEncoded with known PNG data — verify dimensions match
- [ ] T017 Add test: Image::FromEncoded with JPEG data — verify decode and dimensions
- [ ] T018 Add test: Image::FromFile with nonexistent path — returns null/error gracefully
- [ ] T019 Add test: Surface::Create and Flush — verify surface is created with correct dimensions
- [ ] T020 Add test: HardwareBuffer::IsValid returns false for default-constructed buffer
- [ ] T021 Add test: Canvas::DrawImage with Surface::Flush — verify image pixel output

## Phase 7: Golden Test

- [ ] T022 Create `tests/golden/BUILD.bazel` — cc_test target for golden test
- [ ] T023 Create `tests/golden/skia_spike_test.cc` — golden test: render known rect, encode PNG, compare hash against committed baseline

## Phase 8: Public Headers & Validation

- [ ] T024 Create `src/framework/public/include/native_ui/render.h` — re-export Canvas, Paint, Path, Image
- [ ] T025 Update `src/framework/public/include/native_ui/surface.h` — re-export Surface, HardwareBuffer
- [x] T026 Update `src/framework/public/BUILD.bazel` — add `//src/framework/render` and `//src/framework/surface` deps (already present, verified)
- [ ] T027 Run full validation: `bazel build //...` + `bazel test //...`

---

## Dependencies & Execution Order

```
Phase 1 (BUILD) → Phase 2 (Paint/Path) → Phase 3 (HardwareBuffer/Surface)
                                                      │
                                                      ├──→ Phase 4 (Image) ──→ Phase 5 (Canvas)
                                                      │
                                                      ├──→ Phase 6 (Tests) ──→ Phase 7 (Golden)
                                                      │
                                                      └──→ Phase 8 (Public headers + Validation)
```
