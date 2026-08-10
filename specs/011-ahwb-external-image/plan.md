# Implementation Plan: Android AHardwareBuffer ExternalImage

**Branch**: `011-ahwb-external-image` | **Date**: 2026-08-03 | **Spec**: [spec.md](spec.md)

**Input**: Feature specification from `specs/011-ahwb-external-image/spec.md`

## Summary

Implement the Android-side of the `ExternalImage` widget on a **zero-copy GPU closed loop**: an AHardwareBuffer becomes a GPU texture (import, no CPU round-trip) that `ExternalImage` draws, and the Skia canvas itself is hosted on the `AMediaCodec_createInputSurface()` encoder surface (GLES/EGL) so the composed frame is fed back to the encoder and muxed into an MP4. Concretely:

1. Build an AHardwareBuffer utility layer (modeled on `falcon/core/utils/ahwb.cc`, no OpenCV).
2. Wire `Image::FromBuffer()` / `Surface::CreateFromBuffer()` for two backends: **CPU** (raster, Android fallback path) and **GPU** (GLES/EGL, zero-copy).
3. Introduce `RenderContext` (GrDirectContext + EGL display/context/surface) with a factory that builds the canvas target from the encoder input surface (`ANativeWindow*` from `AMediaCodec_createInputSurface`).
4. **Verification closed loop (min API 29, Android device/emulator)**: PNG → RGBA → allocate AHardwareBuffer → `ExternalImage` → draw onto the encoder-input-surface canvas → `eglSwapBuffers` → `AMediaCodec` encodes → `AMediaMuxer` writes `/tmp/external_image.mp4`. Host CI keeps a green build and regression tests only — non-Android implementation points are guarded stubs (`// TODO(android-only)` + immediate return); the real rendering path is exercised on Android only.

## Technical Context

**Language/Version**: C++17

**Build System**: Bazel 6.5.0 (host); Android build via Bazel NDK toolchain (prerequisite, see Constraints)

**Primary Dependencies**: render (Skia `SkImage`/`SkBitmap`), surface (`HardwareBuffer`, `Surface`, `SurfaceFactory`, `RenderContext`), widgets (`ExternalImage`), utils (`PngWriter`), Android NDK `<android/hardware_buffer.h>` (Android-only), Skia GPU `GrDirectContext` / `GrAHardwareBufferUtils` (Android-only), Android EGL (`<EGL/egl.h>`, `<GLES3/gl3.h>`), MediaCodec/MediaMuxer NDK native API (`<media/NdkMediaCodec.h>`, `<media/NdkMediaMuxer.h>`, `<media/NdkMediaFormat.h>` — used by the example only)

**Storage**: N/A (transient buffers; MP4 output file for verification)

**Testing**: Host (macOS/Linux) — Bazel `cc_test` for wrapper/regression contract only (hardware-buffer geometry/equality, host stubs return `nullptr`/`-5` without crash, `Surface::Create` raster unchanged). Android device/emulator — `examples/external_image_demo.cc` for the real AHardwareBuffer → encoder-surface → MP4 closed loop plus `--live` mode (30 Hz / 60 s updates, bounded memory) and `DumpPng` diagnostics.

**Target Platform**: Android 10+ (API 29+, sole implemented platform); macOS ARM64 host (development + CI keeps the build green with guarded stubs)

**Project Type**: C++ library (framework rendering + platform utility layer + EGL/encoder-surface integration + verification example)

**Performance Goals**: 1080p frame, GPU path zero-copy: PNG RGBA upload amortized, texture import + compose + `eglSwapBuffers` within the 16.6ms frame budget; no CPU readback on the encode path. 10,000 successive updates with bounded memory (no leaks).

**Constraints**: C++17, no exceptions. No OpenCV dependency (project uses stb_image/Skia). Android is the **only implemented platform**: every platform-specific implementation point keeps `#if defined(__ANDROID__)` conditional compilation; non-Android branches carry a `// TODO(android-only): ...` comment and return immediately (compile-green host, no host rendering). **Min API 29** — `Image.getHardwareBuffer()` (API 28+) is unconditionally usable, no CPU fallback for decode. Buffer producers retain ownership. **GLES/EGL chosen over Vulkan**: the canvas render target is `eglCreateWindowSurface(AMediaCodec_createInputSurface())`; `GrDirectContext` is built on that same EGL context (single-context rule — see `contracts/render-backend.md`). **MediaCodec/MediaMuxer are native NDK C APIs** (`AMediaCodec`/`AMediaMuxer`/`AMediaFormat`, API 21+; `AMediaCodec_createInputSurface` API 24+ → `ANativeWindow*`): the example drives the encoder and muxer directly, so the whole feature — framework AND example — is C++/JNI-free. Link the Android example against `-lmediandk -landroid -lEGL -lGLESv3`.

**Prerequisites (repo, verified)**: `third_party/skia/BUILD.bazel` currently **excludes `src/gpu/**` and `src/android/**`** → must add a `skia_gpu` cc_library (or extend the existing target) so `GrDirectContext`/`GrAHardwareBufferUtils` link. The repo has no Android Bazel platform/toolchain (no NDK registration in `WORKSPACE`/`platforms/`/`.bazelrc`) → required to build `__ANDROID__` code under Bazel.

**Scale/Scope**: 3 new source files (`ahwb.h/.cc`, `hardware_buffer.cc`, `render_context.h/.cc`), ~4 modified framework files (`hardware_buffer.h`, `surface.h/.cc`, `image.cc`, `external_image.cc`), 1 new example (native `AMediaCodec`/`AMediaMuxer`, no JNI), 1 new test file, BUILD + `native_ui_deps.bzl` updates. No new third-party deps.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Constitution file contains placeholder template — no binding principles defined. All gates PASS.

## Project Structure

### Documentation (this feature)

```text
specs/011-ahwb-external-image/
├── spec.md              # Feature specification
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output
│   ├── ahwb.md          # Android AHardwareBuffer utility contract
│   ├── render-backend.md# CPU/GPU surface + image backend + RenderContext/EGL contract
│   └── media-codec.md   # MediaCodec decode (getOutputImage) + encode (createInputSurface) contract
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code

```text
native_ui/src/framework/surface/
├── hardware_buffer.h        # MODIFY — add dims/stride/format, operator==, FromMemory() (host verification)
├── hardware_buffer.cc       # NEW — factory definitions; Android FromAHardwareBuffer; CPU FromMemory
├── ahwb.h / ahwb.cc         # NEW — Android AHardwareBuffer utility layer (reference AHwbPool, no OpenCV)
├── render_context.h/.cc     # NEW — RenderContext: GrDirectContext + EGL display/context/surface
│                            #        + CreateFromMediaCodecInputSurface() (GLES/EGL, Android)
├── surface.h / surface.cc   # MODIFY — RenderBackend enum; CreateFromBuffer(backend, RenderContext*)
└── surface_factory.cc       # MODIFY — route CreateFromHardwareBuffer through backend selection

native_ui/src/framework/render/
├── image.h / image.cc       # MODIFY — implement Image::FromBuffer (CPU memory/ahwb copy path; GPU texture path)
└── BUILD.bazel              # (no change — surface is already a dep)

native_ui/src/framework/widgets/
└── external_image.cc        # MODIFY — redundant-rebuild guard via HardwareBuffer operator==; CPU backend default

native_ui/examples/
├── external_image_demo.cc   # NEW — verification closed loop (Android only):
│                            #   PNG→AHardwareBuffer→ExternalImage→canvas on encoder input surface→MP4
│                            #   + --live mode (30 Hz / 60 s) + DumpPng diagnostics
├── BUILD.bazel              # MODIFY — add external_image_demo cc_binary (data: photos; Android linkopts)
└── android_media.cc         # NEW — native AMediaCodec encoder + createInputSurface (ANativeWindow) + AMediaMuxer (Android only)

native_ui/tests/
├── external_image_test.cc   # NEW — host contract tests: HardwareBuffer geometry/equality via FromMemory,
│                            #   host stub behavior (FromBuffer/CreateFromBuffer → nullptr, no crash), regression
├── surface_test.cc          # MODIFY — RenderBackend + stub contract tests (host returns nullptr)
├── render_test.cc           # MODIFY — Image::FromBuffer stub contract tests (host returns nullptr)
└── BUILD.bazel              # MODIFY — add external_image_test target

native_ui/third_party/skia/BUILD.bazel   # MODIFY — add skia_gpu target (enable src/gpu/**) — prerequisite
native_ui/native_ui_deps.bzl             # MODIFY — Android NDK toolchain registration (rules_android) + android platform — prerequisite
```

**Structure Decision**: The Android-only platform code lives in `surface/ahwb.*` and `surface/render_context.*` next to `HardwareBuffer`, mirroring falcon's `core/utils/ahwb.cc` but scoped to this repo's conventions (no OpenCV, Skia for pixels, EGL for GPU). The cross-platform widget/render pipeline (`ExternalImage` + `Image::FromBuffer`) stays in `widgets/` and `render/` and is backend-agnostic; backend selection is centralized in `surface`. **Android is the only implemented platform**; every non-Android implementation point is a guarded stub with `// TODO(android-only)` + immediate return, so the host build stays green and the existing raster `Surface::Create(w,h)` path is untouched (FR-012). MediaCodec/MediaMuxer are driven through the **native NDK C API** (`AMediaCodec`/`AMediaMuxer`) isolated in `examples/android_media.cc` — no JNI anywhere. The verification loop is an Android-only example (device: encode to MP4; `--live` mode for updates; `DumpPng` diagnostics) plus host contract/regression unit tests.

## Design

### 1. `HardwareBuffer` wrapper (`surface/hardware_buffer.h` + new `.cc`)

Today `HardwareBuffer` is header-only, holds only a platform handle, and declares factories that have **no definition anywhere** (`FromAHardwareBuffer` etc. are link errors if used). The plan fixes this:

- Add `hardware_buffer.cc` defining all factories.
- Extend the class with cached geometry so downstream code (and `ExternalImage`) avoids re-describing:
  - `int width()/height()`, `size_t row_bytes()`, `int format()` (0 = unknown).
  - `bool operator==(const HardwareBuffer&)` — compares the underlying handle; used by `ExternalImage` to skip redundant per-frame re-conversion.
- Add a platform kind tag: `Memory` (host CPU pixels, reserved — used by wrapper contract tests) | `AHardwareBuffer` (Android) | `Invalid`.
- Factories:
  - `static HardwareBuffer FromAHardwareBuffer(void* buffer)` (Android) — **non-owning** (producer retains lifetime, FR-011). Cached dims/stride lazily filled from `AHardwareBuffer_describe` on first access.
  - `static HardwareBuffer FromMemory(void* pixels, size_t row_bytes, int width, int height)` (all platforms) — **non-owning** CPU-backed buffer. Data-only factory (no rendering): keeps `hardware_buffer.cc` host-testable and reserved for future non-Android use; not used by the Android render path.

### 2. Android AHardwareBuffer utility layer (`surface/ahwb.h/.cc`)

A static-class `AHwb` modeled on `falcon::utils::AHwbPool` (`ahwb.cc`) with the same lock/unlock/describe/allocate/release discipline, minus OpenCV (raw `uint8_t*` RGBA instead of `cv::Mat`):

| Function | Maps to falcon | Notes |
|----------|----------------|-------|
| `Describe(ahwb, w, h, stride, format)` | `AHardwareBuffer_describe` | fills geometry |
| `Lock(ahwb, usage, &data)` / `Unlock(ahwb)` | `AHardwareBuffer_lock/unlock` | same fence=-1, rect=nullptr |
| `Pixels(ahwb, usage, fn)` | `AHwbPool::Pixels` | RAII lock→call→unlock; guarantees unlock on early return |
| `AllocateRgba(w, h)` | `U8C4` allocation half | `AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM`; usage `CPU_READ_OFTEN\|CPU_WRITE_OFTEN\|GPU_SAMPLED_IMAGE\|GPU_COLOR_OUTPUT` (flags support both backends and the encoder surface) |
| `WriteRgba(ahwb, src, src_row_bytes)` | `U8C4` write half | copy per-row honoring **dst stride** (FR-002) |
| `Release(ahwb)` | `AHwbPool::Release` | wrapper around `AHardwareBuffer_release` |
| `ToCpuImage(ahwb, copy=true)` | — (new) | describe→`Lock(CPU_READ_OFTEN)`→`SkBitmap::installPixels` over owned copy→`Unlock` |
| `ToGpuImage(ahwb, GrDirectContext*)` | — (new) | `GrAHardwareBufferUtils::GetBackendTexture` → `SkImages::AdoptTextureFrom` (zero-copy) |
| `DumpPng(ahwb, path)` | `AHwbPool::Dump` | CPU read → Skia encode → PNG (diagnostic, FR-010) |

All methods guarded by `#if defined(__ANDROID__)`; on host the class compiles to stubs returning negative status codes (mirroring falcon's `-1`/`-5` error idiom), keeping host/CI builds green.

### 3. `RenderContext` — GPU/EGL context object (`surface/render_context.h/.cc`)

Replaces the bare `void* gpu_context` with a typed, owned-by-the-app context bundle. **GLES/EGL** is the sole GPU backend.

```cpp
// surface/render_context.h
struct RenderContext {
  GrDirectContext* gr = nullptr;    // Skia GPU context built on egl_ctx (single-context rule)
  void* display = nullptr;          // EGLDisplay
  void* context = nullptr;          // EGLContext
  void* surface = nullptr;          // EGLSurface — from AMediaCodec_createInputSurface() (Android)

  static std::unique_ptr<RenderContext> CreateFromMediaCodecInputSurface(
      ANativeWindow* surface, int width, int height);   // __ANDROID__ only
  void MakeCurrent();               // eglMakeCurrent + gr context current
  void SwapBuffers();               // eglSwapBuffers → frame presented to encoder
  ~RenderContext();                 // destroy surface/context/display
};
```

- Factory flow (Android): get EGLDisplay (default display) → `eglCreateWindowSurface(display, config, native_window)` where `native_window` is the `ANativeWindow*` returned by `AMediaCodec_createInputSurface()` → `eglCreateContext` (GLES 3.x) → `GrGLMakeNativeInterface` → `GrDirectContext::MakeGL` → adopt the EGLSurface.
- **Single-context rule**: the same EGLContext hosts the encoder-surface render target AND all AHardwareBuffer texture imports (`GrAHardwareBufferUtils` runs in `gr`). A second context would force a readback and break zero-copy.
- Host/CI builds: factory returns `nullptr` (guarded stub, `// TODO(android-only)`); callers fall back to `RenderBackend::kCPU` (which on host is also a stub returning `nullptr`).
- Threading: created and used on the render thread; `MakeCurrent()` before any Skia draw; `SwapBuffers()` after flush.

### 4. `Image::FromBuffer` (`render/image.cc`)

Replaces the `return nullptr;` stub (image.cc:100) with backend-aware conversion:

```cpp
static std::unique_ptr<Image> FromBuffer(HardwareBuffer buffer,
    RenderBackend backend = RenderBackend::kCPU, RenderContext* ctx = nullptr);
```

- **CPU backend** (Android; verified via the device demo):
  - AHardwareBuffer kind → `AHwb::ToCpuImage(ahwb, copy=true)` — owned `SkData::MakeWithCopy` honoring `row_bytes` (padding dropped, FR-002); `Image` owns the copy, so the producer can release the buffer afterwards (FR-011).
- **GPU backend** (Android, min API 29): AHardwareBuffer kind + non-null `ctx` → `AHwb::ToGpuImage(ahwb, ctx->gr)` — zero-copy texture import; producer must keep the buffer alive while drawn. Any other combination falls back to CPU, then to `nullptr` (FR-006: unsupported → defined error state, no corrupt output).
- **Host (macOS/Linux)**: `FromBuffer` is a guarded stub — `// TODO(android-only)` + immediate `return nullptr` (build-green, no host rendering; contract tested in `external_image_test.cc`).

### 5. Surface backends (CPU / GPU + encoder surface) (`surface.h/.cc`)

Introduce `enum class RenderBackend { kCPU, kGPU };` and rework `Surface::CreateFromBuffer`:

```cpp
static std::unique_ptr<Surface> CreateFromBuffer(HardwareBuffer buffer,
    RenderBackend backend = RenderBackend::kCPU, RenderContext* ctx = nullptr);
```

- **CPU backend** (Android): `AHwb::Pixels`-style lock with `CPU_READ_OFTEN|CPU_WRITE_OFTEN`; wrap the locked memory as `SkSurfaces::WrapPixels(RGBA8888, w, h, rowBytes)` — a zero-copy raster surface drawing the widget tree straight into buffer memory. `SurfaceImpl` keeps the buffer **locked for the surface's whole lifetime** (RAII `AHwb::ScopedLock`); `Flush()`/`~Surface` unlock (write-back). Stride honored (FR-002).
- **GPU backend (buffer target)**: with non-null `ctx`, `GrAHardwareBufferUtils::GetBackendTexture` → `SkSurfaces::WrapBackendTexture` (usage `GPU_SAMPLED_IMAGE|GPU_COLOR_OUTPUT`). Drawing is GPU-side; `Flush()` submits via `ctx->gr`.
- **GPU backend (encoder-surface target, the closed-loop case)**: canvas hosted on `AMediaCodec_createInputSurface()`. Flow: `RenderContext::CreateFromMediaCodecInputSurface` → `eglMakeCurrent` → wrap the window surface as a render target → draw → `ctx->SwapBuffers()` → encoder dequeues the presented buffer (zero-copy) → `AMediaMuxer` writes MP4.
- `SurfaceFactory::CreateFromHardwareBuffer` forwards `backend`/`ctx`.
- `Surface::Create(w,h)` (plain raster) is unchanged — existing tests keep passing (FR-012).
- **Host (macOS/Linux)**: `CreateFromBuffer` is a guarded stub — `// TODO(android-only)` + immediate `return nullptr` (build-green; contract tested in `surface_test.cc`).

**GPU commitment (decided)**: GLES/EGL is the backend; the encoder-surface target is the concrete on-device verification for the GPU path (PNG-driven compose → MP4), so GPU is no longer "designed but unverified" — it is verified through the encode loop on an API 29+ device.

### 6. `ExternalImage` widget (`widgets/external_image.cc`)

No structural change — the widget already calls `Image::FromBuffer` in `ProcessArg`/`SetBuffer`/`Draw`. Two fixes:

- Guard `Draw()`'s watched-property re-conversion with `operator==`: only rebuild `image_` when the buffer handle actually changed, so a static frame isn't re-copied every draw (FR-003/FR-007).
- Pass `RenderBackend::kCPU` (default) so behavior is deterministic while GPU is opt-in.

### 7. MediaCodec integration contract (`contracts/media-codec.md`)

MediaCodec is consumed through the **native NDK C API** (`<media/NdkMediaCodec.h>`, `<media/NdkMediaMuxer.h>`) — no JNI. Two MediaCodec surfaces exist; only the encoder side is in scope for this feature (decoder surface is documented as the future "video texture" path):

- **Decode (context, min API 29, future)**: `AMediaCodec` (decode) → output `Image` via `AMediaCodec_getOutputImage().getHardwareBuffer()` → `HardwareBuffer::FromAHardwareBuffer` → `Image::FromBuffer(kGPU, ctx)` → zero-copy texture into `ExternalImage`. This is how real video feeds the widget; the PNG-driven demo stands in for it.
- **Encode (in scope)**: `AMediaCodec_createEncoderByType("video/avc")` + `AMediaCodec_configure(...)` → `AMediaCodec_createInputSurface(&window)` (`ANativeWindow*`) → `RenderContext::CreateFromMediaCodecInputSurface(window, w, h)` → canvas on that surface → `SwapBuffers()` → `AMediaCodec_dequeueOutputBuffer` → `AMediaMuxer` → MP4. GLES/EGL only; the EGLSurface is the default framebuffer (`GrGLRenderTargetInfo{fFBOID=0}`). Link `-lmediandk -landroid -lEGL -lGLESv3`.

### 8. Verification closed loop (`examples/external_image_demo.cc` + `android_media.cc` + `tests/external_image_test.cc`)

**Android (min API 29, on device/emulator) — PNG-driven compose → MP4:**

```
1. Image::FromFile("assets/photo/police.png") → decode RGBA (w,h)            [load PNG]
2. AHwb::AllocateRgba(w, h)                                                  [allocate ahwb; usage incl. GPU flags]
3. AHwb::WriteRgba(ahwb, rgba, row_bytes)                                    [write; inject +16B/row padding → FR-002]
4. HardwareBuffer::FromAHardwareBuffer(ahwb)                                 [wrap]
5. ExternalImage(hb, Width{w}, Height{h}) → root                             [feed widget]
6. AMediaCodec_createEncoderByType("video/avc") → configure →                [encoder]
   AMediaCodec_createInputSurface(&window)                                   [ANativeWindow*]
7. RenderContext::CreateFromMediaCodecInputSurface(window, w, h)             [encoder input surface hosts canvas]
   MakeCurrent → WrapBackendRenderTarget → root.Draw → ctx->SwapBuffers()    [compose + present, zero-copy]
8. AMediaCodec_dequeueOutputBuffer → AMediaMuxer → /tmp/external_image.mp4    [encode output]
9. Decode-back the MP4 and pixel-diff vs source; AHwb::Release(ahwb);        [verify + cleanup]
   release encoder/muxer
```

**`--live` mode (Android)**: after the single-frame loop, cycle buffers at 30 Hz for 60 s (alternating frames / animated content) to exercise the update path — the widget stays current, memory stays bounded (FR-003/FR-004, SC-002/003/004). **Diagnostics (Android)**: export the displayed frame via `PngWriter` and `AHwb::DumpPng` for pixel verification (FR-010, SC-006).

**Host CI (contract + regression only)**: `external_image_test.cc` asserts the guarded-stub contract on macOS/Linux — `HardwareBuffer::FromMemory` geometry/`operator==`, `Image::FromBuffer`/`Surface::CreateFromBuffer` return `nullptr` (TODO stubs) without crashing, `ExternalImage::Draw` with no valid buffer no-ops, and 10k successive `SetBuffer` cycles show no memory growth. `Surface::Create(w,h)` raster behavior is unchanged (FR-012).

## Complexity Tracking

> Constitution has no binding principles — no violations to justify.

## Implementation Order (task grouping)

0. **Prerequisites (blocking)**: add `skia_gpu` Bazel target (enable `src/gpu/**`); register Android NDK toolchain + android platform (`native_ui_deps.bzl`/`platforms/`/`.bazelrc`). Without these, no `__ANDROID__`/GPU code can build.
1. **Foundation**: `hardware_buffer.h` extensions + `hardware_buffer.cc`; `ahwb.h/.cc` (Android guarded, host stubs); BUILD updates.
2. **Render integration**: `RenderBackend` enum; `Image::FromBuffer` (CPU ahwb path + GPU path; host stub); `Surface::CreateFromBuffer` CPU/GPU backends; `ExternalImage` guards.
3. **GLES/EGL + RenderContext**: `render_context.h/.cc` with `CreateFromMediaCodecInputSurface(ANativeWindow*)`; GPU texture import + `WrapBackendRenderTarget` paths.
4. **Verification**: `external_image_test.cc` (host contract/regression), `external_image_demo.cc` + `android_media.cc` (Android encode→MP4 via AMediaCodec/AMediaMuxer, `--live` mode, DumpPng), BUILD targets.
5. **Validation**: `bazel test //tests:external_image_test //tests:surface_test //tests:render_test` (host, green with stubs); build `//examples:external_image_demo` for Android and run on an API 29+ device/emulator (MP4 output, decode-back + pixel-diff; `--live` mode; perf check).
