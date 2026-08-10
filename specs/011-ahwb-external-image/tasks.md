# Tasks: Android AHardwareBuffer ExternalImage

**Input**: Design documents from `/specs/011-ahwb-external-image/`

**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/ (ahwb.md, render-backend.md, media-codec.md), quickstart.md

**Tests**: Included — the feature specification mandates verification (Independent Test per story). Android is the **only implemented platform**; host CI validates the guarded-stub contract and regressions, while the real rendering/encode loop is validated on an API 29+ device/emulator.

**Organization**: Tasks are grouped by user story so each story can be implemented and validated independently.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- Bazel workspace root: `native_ui/` (repo lives at `native_ui/native_ui/`). File paths below are relative to the Bazel workspace root (`native_ui/`); BUILD labels are workspace-relative (`//tests:external_image_test`).
- Path prefix used in task descriptions: `native_ui/src/framework/...`, `native_ui/tests/...`, `native_ui/examples/...`, `native_ui/third_party/...`.
- **Platform rule**: every platform-specific implementation point keeps `#if defined(__ANDROID__)`; non-Android branches carry `// TODO(android-only): ...` and return immediately (build-green host, no host rendering).

---

## Phase 1: Setup

**Purpose**: Confirm the green baseline and demo assets before any feature work.

- [X] T001 [P] Verify host baseline: from `native_ui/` run `bazel test //tests:infra_test //tests:render_test //tests:surface_test //tests:widgets_test` on macOS ARM64 and confirm all pass before feature changes.
- [X] T002 [P] Confirm demo asset: verify `native_ui/assets/photo/police.png` exists and is globbed by the `photos` filegroup in `native_ui/assets/photo/BUILD.bazel` (referenced later by `//examples:external_image_demo` data).

**Checkpoint**: Clean baseline — user story work can begin after Foundational.

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Repo-level build prerequisites plus the shared buffer/utility layer every story depends on. No user story can start until this phase is complete.

**⚠️ CRITICAL**: Blocking — the plan's step 0 (skia_gpu + Android NDK toolchain/platform) and the `HardwareBuffer`/`AHwb` layer gate all stories.

- [X] T003 Register Android platform + NDK toolchain: add `rules_android_ndk` http_archive in `native_ui/native_ui_deps.bzl` (bazelbuild/rules_android_ndk @ d5c9d46, the pattern verified in the atlas project) and `android_ndk_repository(name = "androidndk", api_level = 29)` + `bind(name = "android/crosstool", ...)` in `native_ui/WORKSPACE` (tolerant of missing `$ANDROID_NDK_HOME` for host builds); add `android_arm64` platform in `native_ui/platforms/BUILD.bazel` and `--config=android_arm64` (`--incompatible_enable_cc_toolchain_resolution` + `--extra_toolchains=@androidndk//:toolchain_aarch64-linux-android` + Android linkopts) in `native_ui/.bazelrc`. Verified: `bazel build --config=android_arm64 //src/framework/surface:surface` builds for aarch64-linux-android29 (NDK r28.2.13676358; note `$ANDROID_NDK_HOME` in the shell defaults to a stale r25).
- [X] T004 [P] Add `skia_gpu` cc_library in `native_ui/third_party/skia/BUILD.bazel`: include `src/gpu/**` + `src/android/**` sources (plus required include dirs such as `include/gpu`, `src/gpu`, `src/gpu/ganesh`) and GL/linkopts so `GrDirectContext`, `GrAHardwareBufferUtils`, `GrBackendTexture`, and `GrGLRenderTargetInfo` link (plan step 0 prerequisite; GPU code is only referenced from `__ANDROID__` code).
- [X] T005 [P] Extend `HardwareBuffer` in `native_ui/src/framework/surface/hardware_buffer.h`: add `Kind` enum (`Memory`/`AHardwareBuffer`/`Invalid`), cached geometry accessors `width()/height()/row_bytes()/format()` (0 = unknown), `bool operator==(const HardwareBuffer&)` comparing the underlying handle, and `FromMemory()` declaration (data-model.md HardwareBuffer entity).
- [X] T006 Implement `native_ui/src/framework/surface/hardware_buffer.cc` (new): `FromAHardwareBuffer(void*)` is Android-only (non-owning, lazy geometry fill via `AHardwareBuffer_describe` on first access) with a host branch `// TODO(android-only)` + `return {}` (Invalid); `FromMemory(void*, size_t row_bytes, int width, int height)` is a cross-platform **data-only factory** (stores pointer + geometry, no rendering) kept for host wrapper tests and reserved for future non-Android use (depends on T005).
- [X] T007 [P] Create `native_ui/src/framework/surface/ahwb.h` (new): declare static-class `AHwb` with `Describe`, `Lock`, `Unlock`, `Pixels`, `AllocateRgba`, `WriteRgba`, `Release`, `ToCpuImage`, `ToGpuImage`, `DumpPng` per `contracts/ahwb.md`; all guarded by `#if defined(__ANDROID__)` with host stubs returning `-5`.
- [X] T008 Implement `native_ui/src/framework/surface/ahwb.cc` (new): `AHardwareBuffer_lock/unlock/describe/allocate/release` discipline with fence `-1`/`nullptr` rect; `AllocateRgba` uses `R8G8B8A8_UNORM` with usage `CPU_READ_OFTEN|CPU_WRITE_OFTEN|GPU_SAMPLED_IMAGE|GPU_COLOR_OUTPUT`; `WriteRgba`/`ToCpuImage` honor destination stride; `Pixels` RAII lock→fn→unlock; `ToGpuImage` wraps via the actual Skia API in this version — `GrAHardwareBufferUtils::MakeGLBackendTexture` + `SkImages::BorrowTextureFrom` (contract's `GetBackendTexture` does not exist in the vendored Skia); host branch = `// TODO(android-only)` stubs returning `-5` (depends on T004, T007).
- [X] T009 Update `native_ui/src/framework/surface/BUILD.bazel`: glob already picks up the new `hardware_buffer.cc`/`ahwb.cc`; add `@skia//:skia_gpu` to `deps` via `select()` (Android builds only, so host stays on `@skia//:skia`) and Android `linkopts` `-lEGL -lGLESv3` (depends on T004, T006, T008).

**Checkpoint**: Foundation ready — `HardwareBuffer`/`AHwb` exist, Android platform + GPU toolchains link; user stories can now be implemented.

---

## Phase 3: User Story 1 - Display an external image buffer on Android (Priority: P1) 🎯 MVP

**Goal**: A producer-supplied AHardwareBuffer bound to `ExternalImage` renders its content correctly on Android — CPU owned-copy path and zero-copy GPU (GLES/EGL) path, including the encoder-surface closed loop.

**Independent Test**: Android (API 29+) — run `//examples:external_image_demo`, decode `/tmp/external_image.mp4`, pixel-diff vs source PNG (SC-001, SC-006). Host — `bazel test //tests:external_image_test` validates the guarded-stub contract (no crash, FR-005/FR-012).

### Tests for User Story 1 (write FIRST, ensure they FAIL before implementation)

> Today `Image::FromBuffer` returns `nullptr` (image.cc:100), `Surface::CreateFromBuffer` returns a fixed 1024x768 raster (surface.cc:28), and no `RenderBackend`/`RenderContext` exist — the Android behavior cannot be asserted on host, so these tests pin the stub contract and regressions.

- [ ] T010 [P] [US1] Write host contract tests `native_ui/tests/external_image_test.cc` (new): `HardwareBuffer::FromMemory` geometry (`width/height/row_bytes`) and `operator==` (same handle equal, different not); `ExternalImage::Draw` with no/empty buffer no-ops without crash (FR-005); `Image::FromBuffer` returns `nullptr` on host (stub contract).
- [ ] T011 [P] [US1] Add `Image::FromBuffer` stub-contract tests in `native_ui/tests/render_test.cc`: on host the function returns `nullptr` for valid (`FromMemory`) and invalid buffers without crashing (documents the `// TODO(android-only)` contract; FR-006).
- [ ] T012 [P] [US1] Add `Surface::CreateFromBuffer` + `RenderBackend` tests in `native_ui/tests/surface_test.cc`: host stub returns `nullptr` for any buffer; `RenderBackend` defaults to `kCPU`; `Surface::Create(w,h)` raster path unchanged (FR-012).

### Implementation for User Story 1

- [ ] T013 [P] [US1] Update `native_ui/src/framework/surface/surface.h`: add `enum class RenderBackend { kCPU, kGPU };`, forward-declare `RenderContext`, change `CreateFromBuffer` to `CreateFromBuffer(HardwareBuffer, RenderBackend = kCPU, RenderContext* = nullptr)` per `contracts/render-backend.md`.
- [ ] T014 [US1] Implement `Surface::CreateFromBuffer` in `native_ui/src/framework/surface/surface.cc`: Android CPU backend wraps locked buffer memory via `SkSurfaces::WrapPixels(RGBA8888, w, h, rowBytes)` holding the buffer locked for the surface lifetime (RAII `AHwb` lock, unlock on `Flush()`/destructor); GPU buffer-target via `GrAHardwareBufferUtils` + `SkSurfaces::WrapBackendTexture`; GPU encoder-surface target via `SkSurfaces::WrapBackendRenderTarget(ctx->gr, GrGLRenderTargetInfo{fFBOID=0, ...})`; kGPU-without-ctx falls back to CPU then `nullptr`; invalid → `nullptr`; host branch = `// TODO(android-only)` + `return nullptr` (depends on T013, T009).
- [ ] T015 [P] [US1] Update `Image::FromBuffer` declaration in `native_ui/src/framework/render/image.h` to `FromBuffer(HardwareBuffer, RenderBackend = kCPU, RenderContext* = nullptr)`; include `surface.h` for `RenderBackend`/`RenderContext` (render BUILD already deps surface) (depends on T013's `RenderBackend`).
- [ ] T016 [US1] Implement `Image::FromBuffer` in `native_ui/src/framework/render/image.cc` (replaces the `nullptr` stub): Android AHardwareBuffer kind → CPU path `AHwb::ToCpuImage(copy=true)` (owned copy honoring `row_bytes`, producer may release, FR-011) or GPU path `AHwb::ToGpuImage(ahwb, ctx->gr)` when `backend == kGPU` + non-null `ctx` (zero-copy); format validation → `nullptr` for unsupported (FR-006); kGPU-without-ctx / Memory kind falls back to CPU then `nullptr`; host branch = `// TODO(android-only)` + `return nullptr` (depends on T014, T015, T017, T008).
- [ ] T017 [P] [US1] Create `native_ui/src/framework/surface/render_context.h` (new): `struct RenderContext { GrDirectContext* gr; void* display; void* context; void* surface; }` with `CreateFromMediaCodecInputSurface(ANativeWindow* surface, int w, int h)`, `MakeCurrent()`, `SwapBuffers()`, `~RenderContext()`; `#if defined(__ANDROID__)` with host factory returning `nullptr` per `contracts/render-backend.md`.
- [ ] T018 [US1] Implement `native_ui/src/framework/surface/render_context.cc` (new): EGL display → `eglCreateWindowSurface(display, config, surface)` where `surface` is the `ANativeWindow*` from `AMediaCodec_createInputSurface()` → GLES 3.x context → `GrGLMakeNativeInterface` → `GrDirectContext::MakeGL` → adopt the EGLSurface; `MakeCurrent`/`SwapBuffers` (`eglSwapBuffers`)/cleanup; single-context invariant enforced; host branch = `// TODO(android-only)` factory returning `nullptr` (depends on T004, T017).
- [ ] T019 [P] [US1] Update `ExternalImage` in `native_ui/src/framework/widgets/external_image.cc`: pass `RenderBackend::kCPU` explicitly to every `Image::FromBuffer` call in `ProcessArg`/`SetBuffer`/`Draw` so behavior is deterministic while GPU is opt-in (plan section 6).
- [ ] T020 [US1] Add `external_image_test` cc_test target in `native_ui/tests/BUILD.bazel` with deps `@native_ui//src/framework/widgets`, `@native_ui//src/framework/render`, `@native_ui//src/framework/surface`, `@com_google_googletest//:gtest_main` (depends on T010 file existing).
- [ ] T021 [P] [US1] Create native media helper `native_ui/examples/android_media.cc` (new): `AMediaCodec` H.264 encoder (`AMediaCodec_createEncoderByType("video/avc")` + `AMediaCodec_configure`) + `AMediaCodec_createInputSurface(&window)` (`ANativeWindow*`, API 24+), `AMediaCodec_dequeueOutputBuffer`, and `AMediaMuxer` MP4 writer; `__ANDROID__`-guarded, no JNI, no exceptions (plan section 7 encode egress).
- [ ] T022 [US1] Create demo `native_ui/examples/external_image_demo.cc` (new, Android only): `Image::FromFile("assets/photo/police.png")` → `AHwb::AllocateRgba(w,h)` → `AHwb::WriteRgba` (inject +16B/row padding, FR-002) → `HardwareBuffer::FromAHardwareBuffer` → `ExternalImage` → `AMediaCodec_createInputSurface` → `RenderContext::CreateFromMediaCodecInputSurface(window, w, h)` → `MakeCurrent` → `WrapBackendRenderTarget` → `root.Draw` → `gr->flush` → `SwapBuffers()` → `AMediaMuxer` → `/tmp/external_image.mp4`; cleanup `AHwb::Release` + encoder/muxer release; decode-back the MP4 and pixel-diff vs source (depends on T021, T016, T018).
- [ ] T023 [US1] Add `//examples:external_image_demo` cc_binary in `native_ui/examples/BUILD.bazel` (srcs: `external_image_demo.cc`, `android_media.cc`; deps: `@native_ui//src/framework/public:native_ui`, `@native_ui//src/framework/surface`, `@native_ui//src/framework/utils`, `@skia//:skia`, `@skia//:skia_gpu`; data: `@native_ui//assets/photo:photos`; Android `linkopts`: `-lmediandk -landroid -lEGL -lGLESv3`) (depends on T021, T022).
- [ ] T024 [US1] Update `SurfaceFactory::CreateFromHardwareBuffer` in `native_ui/src/framework/surface/surface_factory.h` and `native_ui/src/framework/surface/surface_factory.cc` to accept `RenderBackend` + `RenderContext*` and forward them to `Surface::CreateFromBuffer` (plan section 5) (depends on T014).

**Checkpoint**: User Story 1 fully functional and testable — static AHardwareBuffer displays correctly on Android (CPU fallback + GPU zero-copy), verified via the encode-to-MP4 closed loop.

---

## Phase 4: User Story 2 - Live-updating external frames (Priority: P2)

**Goal**: A watchable buffer property drives continuous updates (live preview/playback); frames reflect the latest buffer on the next rendered frame; the last frame persists when the producer stops; memory stays bounded.

**Independent Test**: Android — demo `--live` mode runs 30 Hz / 60 s with updates tracked and bounded memory (FR-003/004, SC-002/003/004). Host — `bazel test //tests:external_image_test` 10,000-iteration `SetBuffer` loop shows no memory growth / no crash on the stub path (FR-004).

### Tests for User Story 2 (write FIRST, ensure they FAIL before implementation)

- [ ] T025 [US2] Add host update-path tests in `native_ui/tests/external_image_test.cc`: 10,000 successive `SetBuffer` cycles (including invalid/empty buffers) keep memory bounded and never crash on the host stub path (FR-004/SC-004, FR-005); repeated `SetBuffer` with the same handle is a no-op (guard contract).

### Implementation for User Story 2

- [ ] T026 [US2] Implement redundant-rebuild guard in `native_ui/src/framework/widgets/external_image.cc`: in `ProcessArg`/`SetBuffer`/`Draw`, use `HardwareBuffer::operator==` to skip `Image::FromBuffer` re-conversion when the underlying handle is unchanged, so a static frame is not re-copied every draw and 30fps updates are cheap (FR-003/FR-007; depends on T005, T019).
- [ ] T027 [US2] Add `--live` mode to `native_ui/examples/external_image_demo.cc`: cycle buffers at 30 Hz for 60 s (alternating frames / animated content) exercising the watchable path — the widget tracks updates, the app stays responsive, and memory stays bounded on device (FR-003/FR-004/FR-008, SC-002/003/004; depends on T022, T026).

**Checkpoint**: User Stories 1 AND 2 work independently.

---

## Phase 5: User Story 3 - Producer utilities and diagnostics for external buffers (Priority: P3)

**Goal**: Producers can create hardware-backed buffers from in-memory pixels, export the currently displayed frame to a viewable PNG, and get a defined error state for unsupported formats (no corrupt output).

**Independent Test**: Android — demo exports the frame/source buffer as PNG; pixel-match verified (FR-010, SC-006). Host — invalid/unsupported/zero-area buffers yield `nullptr` + no render, no crash (FR-006, SC-005).

### Tests for User Story 3 (write FIRST, ensure they FAIL before implementation)

- [ ] T028 [US3] Add error-state tests in `native_ui/tests/external_image_test.cc`: unsupported/invalid/zero-area buffer → `Image::FromBuffer` returns `nullptr` and `ExternalImage::Draw` no-ops without crash (FR-006, FR-005, SC-005); `HardwareBuffer::FromMemory` data factory exposes correct geometry for producers (FR-009 data-level).

### Implementation for User Story 3

- [ ] T029 [US3] Implement diagnostic export in `native_ui/examples/external_image_demo.cc`: after drawing, export the displayed frame via `PngWriter::Write` and the source buffer via `AHwb::DumpPng` for pixel verification (FR-010, SC-006; depends on T022, T008).
- [ ] T030 [P] [US3] Harden format validation in `native_ui/src/framework/render/image.cc` (`Image::FromBuffer`) and `native_ui/src/framework/surface/ahwb.cc` (`ToCpuImage`/`ToGpuImage`): reject buffers whose `format()` is neither unknown nor `R8G8B8A8_UNORM`, returning `nullptr` — defined error state, never corrupt output; zero-area buffers render nothing (FR-006; depends on T016, T008).

**Checkpoint**: All user stories independently functional.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Documentation, full validation, and performance confirmation across all stories.

- [ ] T031 [P] Update framework docs and `native_ui/CHANGELOG.md`: document `ExternalImage`, `HardwareBuffer` kind/factories, `RenderBackend`, `AHwb`, and `RenderContext` for framework users; note Android-only scope and `// TODO(android-only)` stubs.
- [ ] T032 [P] Run full host validation from `native_ui/`: `bazel test //tests:external_image_test //tests:surface_test //tests:render_test //tests:widgets_test` — all green (stubs compile, FR-012: no regression to existing raster path).
- [ ] T033 [P] Android device/emulator validation (API 29+): `bazel build --config=android_arm64 //examples:external_image_demo`, push binary + `assets/photo/police.png` via `adb`, run, pull and decode `/tmp/external_image.mp4`, pixel-diff vs source PNG — closes the zero-copy GPU loop end-to-end.
- [ ] T034 [P] Performance check (FR-007): on device, confirm a 1080p frame (texture import + compose + `SwapBuffers`) completes within the 16.6ms budget and memory stays bounded over the `--live` 60 s run (SC-003/004).
- [ ] T035 [P] Validate `specs/011-ahwb-external-image/quickstart.md`: run each documented command and confirm the described behavior matches (host tests + Android demo).

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — baseline + asset confirmation, can start immediately.
- **Foundational (Phase 2)**: Depends on Setup. **BLOCKS all user stories** (Android platform/NDK, skia_gpu, `HardwareBuffer`, `AHwb`).
- **User Stories (Phase 3+)**: All depend on Foundational. US2 and US3 build on US1 (live update and diagnostics sit on the display path), so stories run in priority order P1 → P2 → P3.
- **Polish (Final Phase)**: Depends on all desired user stories being complete.

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) — no dependency on other stories. MVP scope.
- **User Story 2 (P2)**: Depends on US1 (live updates render through the US1 display path).
- **User Story 3 (P3)**: Depends on US1 (uses `Image::FromBuffer`/`Surface` from US1 and the demo from T022).

### Within Each User Story

- Tests MUST be written first and FAIL before implementation (T010-T012, T025, T028).
- Headers/declarations before implementations (`T013 → T014`, `T015 → T016`, `T017 → T018`).
- Implementations before integration (US1: surface → image → widget → media helper → demo → BUILD).
- Story complete and validated before moving to the next priority.

### Parallel Opportunities

- Setup: T001, T002 run in parallel.
- Foundational: T003, T004 run in parallel (different files); T005/T006, T007/T008 are header→impl pairs but T005 and T007 (and T004, T003) are mutually parallel.
- US1: tests T010/T011/T012 run in parallel; declarations T013/T015/T017 run in parallel; simple edits T019/T021 run in parallel; T020/T023/T024 are BUILD/factory wiring after their sources exist.
- US2 and US3: sequential within their phases (single-file test/impl extensions).
- Polish: T031-T035 all run in parallel.

---

## Parallel Example: User Story 1

```bash
# Launch all US1 tests together (stub-contract tests, green only after stubs land):
Task: "Write host contract tests native_ui/tests/external_image_test.cc"
Task: "Add Image::FromBuffer stub-contract tests in native_ui/tests/render_test.cc"
Task: "Add Surface::CreateFromBuffer + RenderBackend tests in native_ui/tests/surface_test.cc"

# Launch all US1 declarations together:
Task: "Update Surface::CreateFromBuffer declaration in surface.h"
Task: "Update Image::FromBuffer declaration in image.h"
Task: "Create RenderContext declaration (ANativeWindow) in render_context.h"
```

```bash
# Launch all Polish tasks together:
Task: "Update framework docs and CHANGELOG.md"
Task: "Run full host validation: bazel test //tests:..."
Task: "Android device validation (MP4 decode + pixel-diff)"
Task: "Performance check (FR-007)"
Task: "Validate quickstart.md commands"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (baseline green + assets).
2. Complete Phase 2: Foundational (CRITICAL — Android platform/NDK, skia_gpu, `HardwareBuffer`, `AHwb`).
3. Complete Phase 3: User Story 1 (Android CPU + GPU backends, encoder-surface closed loop).
4. **STOP and VALIDATE**: `bazel test //tests:external_image_test` green on host (stubs); Android demo produces a decodable, pixel-correct MP4.
5. Demo/deliver if ready — this is the feature's core value (buffers finally render content).

### Incremental Delivery

1. Complete Setup + Foundational → foundation ready.
2. Add User Story 1 → validate independently (host stub tests + device MP4) → MVP delivered.
3. Add User Story 2 → validate independently (`--live` device run + memory tests).
4. Add User Story 3 → validate independently (diagnostics export + error-state tests).
5. Final Polish: docs, full CI, device perf. Each story adds value without breaking prior stories.

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together (T003/T004 split; T005/T007 split).
2. Once Foundational is done: Developer A implements US1 surface/image/RenderContext; Developer B writes US1 tests + media helper/demo; Developer C owns US2/US3 follow-ups.
3. Stories integrate and validate in priority order.

---

## Notes

- [P] tasks = different files, no dependencies.
- [Story] label maps the task to its user story for traceability.
- Every user story is independently completable: host CI validates the guarded-stub contract (`// TODO(android-only)` + immediate return), and Android device/emulator validates real rendering (CPU fallback + GPU zero-copy encode loop).
- Verify tests fail before implementing where meaningful on host; Android-only behavior (T022/T027/T029) is validated by the device run in T033/T034.
- Commit after each task or logical group.
- MediaCodec/MediaMuxer are consumed via the **native NDK C API** (`AMediaCodec`/`AMediaMuxer`, no JNI); the decode ingress (`AMediaCodec_getOutputImage`) is documented in `contracts/media-codec.md` as a future path — OUT OF SCOPE here, requires no new framework machinery.
- Avoid: vague tasks, same-file parallel edits (external_image_test.cc, external_image.cc, image.cc, surface.cc, surface.h, examples/BUILD.bazel are each touched by sequential tasks only).
