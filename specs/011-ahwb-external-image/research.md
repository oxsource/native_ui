# Research: Android AHardwareBuffer ExternalImage

**Date**: 2026-08-03

## Decisions

### Android AHardwareBuffer Utility Layer: Reuse falcon AHwbPool Pattern (no OpenCV)

- **Decision**: Build a small Android-only `AHwb` utility class in `surface/ahwb.h/.cc` modeled directly on `falcon/core/utils/ahwb.cc` — same `AHardwareBuffer_lock/unlock/describe/allocate/release` discipline, same `Pixels()` lock→callback→unlock pattern, same negative-status-code error idiom — but operating on raw `uint8_t*` RGBA instead of `cv::Mat`.
- **Rationale**: `ahwb.cc` is a proven, battle-tested reference for exactly this platform API (correct usage flags, `-1` fence / `nullptr` rect, stride-aware row copies). Dropping OpenCV keeps the dependency set consistent with this repo (stb_image/Skia only) and matches the C++17/no-exceptions constraint. The `Pixels()` RAII pattern guarantees unlock even when the producer callback returns early.
- **Alternatives considered**: OpenCV-based copy of `AHwbPool` verbatim (rejected: introduces a heavyweight dependency this project does not use); direct `AHardwareBuffer_*` calls scattered across `image.cc`/`surface.cc` (rejected: no reuse, error handling duplicated); AndroidX/Skia `GrAHardwareBufferUtils` only (rejected: that is GPU-only; the CPU path still needs lock/unlock).

### CPU Surface Backend as the Verified v1 Path

- **Decision**: The primary, tested backend is CPU: lock the AHardwareBuffer (or wrap host memory), render the widget tree into a zero-copy raster surface (`SkSurfaces::WrapPixels`) that draws directly into buffer memory, and unlock on `Flush()`/destruction.
- **Rationale**: Matches the reference implementation's CPU-first nature, requires no GPU context (this repo has none today), runs on host CI via the memory-backed `HardwareBuffer`, and satisfies all functional requirements (stride handling, no leaks, responsive updates) without embedding complexity. `WrapPixels` is zero-copy, so there is no extra memcpy beyond the producer's original write.
- **Alternatives considered**: Raster-to-temp-then-blit-on-unlock (rejected: extra copy); GPU-only (rejected: unreachable without a context).

### GPU Surface Backend: GLES/EGL, Zero-Copy, Committed (encoder-surface verification)

- **Decision**: `RenderBackend::kGPU` uses **GLES/EGL** only: import AHardwareBuffer as a `GrBackendTexture` (`GrAHardwareBufferUtils::GetBackendTexture` → `SkImages::AdoptTextureFrom` / `SkSurfaces::WrapBackendTexture`), and host the canvas on `MediaCodec.createInputSurface()` (`eglCreateWindowSurface` → `SkSurfaces::WrapBackendRenderTarget`). Requires a non-null `RenderContext`; falls back to CPU otherwise. Compiled under `__ANDROID__`.
- **Rationale**: AHardwareBuffer is designed for zero-copy GPU interop, and the encoder-input surface is the one place the GPU path gets **on-device verification** — the PNG-driven demo composes onto that surface and encodes an MP4. GLES/EGL is the natural pairing with `createInputSurface()` (window surface = default framebuffer) and the best-documented Android path.
- **Alternatives considered**: Vulkan-only import (rejected: needs the repo to own a Vulkan device/context; EGL chosen); GPU designed-but-unverified (rejected: user requires the loop to close on-device).
- **Caveat**: requires the `skia_gpu` Bazel target (repo currently excludes `src/gpu/**`) and an Android NDK toolchain — both are explicit prerequisites (step 0).

### Canvas Hosted on the MediaCodec Encoder Surface (`createInputSurface()`)

- **Decision**: The Skia canvas that composites `ExternalImage` (and UI) is **hosted on `MediaCodec.createInputSurface()`** — the surface MediaCodec provides for feeding an encoder. Render → flush → `eglSwapBuffers` → the encoder dequeues the presented buffer, **zero-copy**, and `MediaMuxer` writes an MP4.
- **Rationale**: This is the literal "MediaCodec-provided surface hosts the Skia canvas" architecture: decode is the future ingress (getOutputImage→AHardwareBuffer), the canvas is the compositor, and the encoder surface is the egress. It closes the loop end-to-end on the GPU with no CPU round-trip.
- **Alternatives considered**: Canvas on a SurfaceView/display surface while sampling video texture (rejected: that is the decode-display path, not the encode closed loop the user specified); drawing UI onto the decoder's SurfaceTexture (rejected: FBO-overwrite would destroy the video frame).

### `RenderContext`: Typed GPU/EGL Context Bundle (replaces `void* gpu_context`)

- **Decision**: Introduce `RenderContext { GrDirectContext* gr; EGLDisplay display; EGLContext context; EGLSurface surface; }` with `CreateFromMediaCodecInputSurface(jobject)` — one object carrying the Skia context AND the EGL handles the encoder-surface canvas needs. `Image::FromBuffer`/`Surface::CreateFromBuffer` take `RenderContext*` instead of `void*`.
- **Rationale**: A bare `void*` cannot carry the EGLSurface/context needed to host the canvas; typing it encodes the single-context invariant (imports and render target share one EGLContext → zero-copy) at the type level.
- **Alternatives considered**: Keep `void*` and pass EGL handles separately (rejected: untyped, error-prone, splits a single-context invariant across parameters).

### Min API 29 (Android 10): No Decode Fallback Needed

- **Decision**: Target **min API 29**. `Image.getHardwareBuffer()` (API 28+) is then unconditionally usable for zero-copy decode ingress; `createInputSurface()` (API 19+) and EGL are available.
- **Rationale**: Removes the CPU fallback branch from the decode side — every supported device takes the zero-copy GPU path. Matches typical modern A/V minimums.
- **Alternatives considered**: min API 26 (rejected: would need CPU fallback for `getHardwareBuffer()` availability and older buffer semantics).

### Verification Closed Loop: PNG-Driven Compose → MP4 (device) + Host gtest (CPU)

- **Decision**: One pipeline, two environments. `examples/external_image_demo.cc` on Android (API 29+) runs: PNG → RGBA → `AHwb::AllocateRgba` → `WriteRgba` → `HardwareBuffer::FromAHardwareBuffer` → `ExternalImage` → draw onto the encoder-input-surface canvas (`RenderContext::CreateFromMediaCodecInputSurface` → `WrapBackendRenderTarget` → `SwapBuffers`) → MediaCodec H.264 → `MediaMuxer` → `/tmp/external_image.mp4`. A CPU `Surface` snapshot is also exported as PNG for cross-checking. `tests/external_image_test.cc` runs the identical widget→surface→PNG path on host CI using `HardwareBuffer::FromMemory`.
- **Rationale**: The real AHardwareBuffer + encoder surface exist only on Android; the widget/surface pipeline must be CI-verifiable on macOS. The memory-backed buffer exercises the same conversion, stride, drawing, and export code, so CI catches regressions while the device validates zero-copy end-to-end (pixel-diff + decode-back the MP4).
- **Alternatives considered**: Android-only example (rejected: no CI coverage); real MediaCodec decode→compose→encode as the demo (rejected for this feature — user chose PNG-driven; decode remains the documented future ingress in `contracts/media-codec.md`); mocking `AHardwareBuffer` (rejected: tests a fiction).

### `HardwareBuffer` Wrapper: Add Geometry + Kind + Equality, Owned-Copy Semantics

- **Decision**: Extend `HardwareBuffer` with cached `width/height/row_bytes/format`, a platform `kind` (Memory | AHardwareBuffer | Invalid), `operator==` on the handle, and `FromMemory()`; implement the previously-undefined factories in a new `hardware_buffer.cc`. All wrapper ownership is **non-owning** — the framework copies what it renders (FR-011) and never releases a producer's buffer.
- **Rationale**: The current header declares factories with no definitions (link errors on use) and stores no geometry, forcing callers to re-describe buffers. Geometry caching avoids that; `operator==` lets `ExternalImage` skip redundant per-draw re-conversion; `FromMemory` is the smallest host-side stand-in for the platform object that makes the pipeline CI-testable.
- **Alternatives considered**: Keep header-only and add geometry only to `Image` (rejected: geometry is a buffer property, needed by surface too); pass ownership into the framework (rejected: violates producer-ownership contract in spec FR-011).
