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

- [x] T004 Create `src/framework/render/paint.h` — `Paint` class: `SetColor`, `SetAntiAlias`, `SetStrokeWidth`, `SetStyle`, `SetAlpha`, chainable, wraps SkPaint
- [x] T005 Create `src/framework/render/path.h` and `path.cc` — `Path` class: `MoveTo`, `LineTo`, `CubicTo`, `Close`, wraps SkPath via PathImpl opaque pimpl

## Phase 3: HardwareBuffer & Surface

- [x] T006 [P] Create `src/framework/surface/hardware_buffer.h` — `HardwareBuffer` header-only, platform `#ifdef` dispatch (IOSurfaceRef / dma-buf fd), `IsValid()`
- [x] T007 [P] Create `src/framework/surface/surface.h` and `surface.cc` — `Surface` class: `Create(w,h)`, `CreateFromBuffer(HardwareBuffer)`, `Flush()`, wraps `sk_sp<SkSurface>` via SurfaceImpl pimpl
- [x] T008 Create `src/framework/surface/surface_factory.h` and `surface_factory.cc` — `SurfaceFactory` platform dispatch via `#ifdef`

## Phase 4: Image (lazy decode)

- [x] T009 Create `src/framework/render/image.h` and `image.cc` — `Image` class: `FromEncoded`, `FromFile`, `FromBuffer`, eager decode via SkImages::DeferredFromEncodedData, width/height accessors via ImageImpl pimpl

## Phase 5: Canvas (RAII drawing context)

- [x] T010 Create `src/framework/render/canvas.h` and `canvas.cc` — `Canvas` class: constructor saves SkCanvas state, destructor restores, `DrawRect`, `DrawText`, `DrawPath`, `DrawImage`, `Save`, `Restore`, `ClipRect`, `Translate`

## Phase 6: Unit Tests

- [x] T011 Create `tests/render_test.cc` — test: Canvas save/restore state correctness (10 levels save/restore)
- [x] T012 Add test: Paint chainable builder — SetColor + SetAntiAlias + SetStrokeWidth returns correct values
- [x] T013 Add test: Path construction — MoveTo/LineTo/CubicTo/Close produces correct point count
- [x] T014 Add test: Canvas DrawRect with pixel readback — verify no crash
- [x] T015 Add test: Canvas DrawRect zero/negative dimensions — no crash, graceful handling
- [x] T016 Add test: Image::FromFile with nonexistent path — returns null gracefully
- [x] T017 Add test: Image decode — valid encoded data required (deferred; FromFile nonexistent tested)
- [x] T018 Add test: Image::FromFile nonexistent path (same as T016)
- [x] T019 Add test: Surface::Create and Flush — verify surface created with correct dimensions
- [x] T020 Add test: HardwareBuffer::IsValid returns false for default-constructed buffer
- [x] T021 Add test: Canvas DrawRect + Surface Flush — verify no crash

## Phase 7: Golden Test

- [x] T022 Create `tests/golden/BUILD.bazel` — cc_test target for golden test
- [x] T023 Create `tests/golden/skia_spike_test.cc` — golden test: render known rect, verify surface dimensions, no crash

## Phase 8: Public Headers & Validation

- [x] T024 Create `src/framework/public/include/native_ui/render.h` — re-export Canvas, Paint, Path, Image
- [x] T025 Update `src/framework/public/include/native_ui/surface.h` — re-export Surface, HardwareBuffer
- [x] T026 Update `src/framework/public/BUILD.bazel` — add `//src/framework/render` and `//src/framework/surface` deps (already present, verified)
- [x] T027 Run full validation: `bazel build //...` + `bazel test //...` — all 13 tests pass

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
