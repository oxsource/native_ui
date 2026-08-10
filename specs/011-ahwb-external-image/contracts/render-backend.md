# Contract: CPU / GPU Render Backend Selection

**Scope**: How `Image::FromBuffer` and `Surface::CreateFromBuffer` choose a rendering backend, and the ownership/lifetime rules each backend implies. **GPU backend = GLES/EGL, min API 29, zero-copy. Android is the only implemented platform; host builds are guarded stubs (`// TODO(android-only)` + immediate return).**

## Backend Enum

```cpp
enum class RenderBackend { kCPU, kGPU };
```

`kCPU` is the default and the Android fallback backend. `kGPU` is Android-only, GLES/EGL, and requires a non-null `RenderContext`. On host builds both backends are unimplemented stubs.

## Context Object (replaces `void* gpu_context`)

```cpp
// surface/render_context.h
struct RenderContext {
  GrDirectContext* gr;      // Skia GPU context on the shared EGL context
  void* display;            // EGLDisplay
  void* context;            // EGLContext (GLES 3.x)
  void* surface;            // EGLSurface — eglCreateWindowSurface(AMediaCodec_createInputSurface() → ANativeWindow*)

  static std::unique_ptr<RenderContext> CreateFromMediaCodecInputSurface(
      ANativeWindow* surface, int width, int height);   // __ANDROID__ only; nullptr on failure/host
  void MakeCurrent();
  void SwapBuffers();                               // eglSwapBuffers → presents to encoder
};
```

**Single-context rule (zero-copy invariant)**: one EGLContext hosts both the render target and every AHardwareBuffer texture import. Skia's `GrDirectContext` (`gr`) is built on that same context. A second context forces a readback — forbidden on the GPU path.

## Signatures

```cpp
// image.h
static std::unique_ptr<Image> FromBuffer(HardwareBuffer buffer,
    RenderBackend backend = RenderBackend::kCPU,
    RenderContext* ctx = nullptr);

// surface.h
static std::unique_ptr<Surface> CreateFromBuffer(HardwareBuffer buffer,
    RenderBackend backend = RenderBackend::kCPU,
    RenderContext* ctx = nullptr);
```

## Backend Selection Rules

| Requested | Buffer kind | ctx | Result |
|-----------|-------------|-----|--------|
| kCPU | Memory | — | Android: reserved (Memory buffers are not produced on Android; falls through to nullptr). Host: stub → `nullptr`. |
| kCPU | AHardwareBuffer | — | `AHwb::ToCpuImage(copy=true)`; CPU surface locking buffer for surface lifetime. |
| kGPU | AHardwareBuffer | non-null | Zero-copy backend-texture image/surface (`GrAHardwareBufferUtils`, GLES/EGL). |
| kGPU | AHardwareBuffer | null | **Fall back to kCPU**; if CPU unavailable, `nullptr`. |
| kGPU | Memory | — | Fall back to kCPU (memory has no GPU interop); on host → stub `nullptr`. |
| any | Invalid | — | `nullptr` (no crash; FR-005/FR-006). |

**Encoder-surface target**: `Surface` may be created directly on `ctx->surface` (from `AMediaCodec_createInputSurface()`) via `SkSurfaces::WrapBackendRenderTarget(ctx->gr, GrGLRenderTargetInfo{fFBOID=0, ...})` — the default framebuffer of the window surface. Draw → `ctx->gr->flush()` → `ctx->SwapBuffers()`.

## Ownership & Lifetime

- The framework **never owns** a producer's buffer (FR-011). CPU paths copy what they render; the resulting `Image`/`Surface` is independent of the producer.
- CPU `Surface::CreateFromBuffer` holds the buffer **locked** for the surface's lifetime (RAII `AHwb::ScopedLock`); unlock on `Flush()`/destruction. Producer must not release a buffer while a CPU surface backed by it is alive.
- GPU `Surface`/`Image` reference the buffer's texture; the producer keeps the buffer alive and the `RenderContext` current while they are in use.

## Platform Rules

- `__ANDROID__` (API 29+): full kCPU + kGPU (GLES/EGL). kGPU requires an app-supplied `RenderContext`.
- Host (macOS/Linux): all `Image::FromBuffer` / `Surface::CreateFromBuffer` / `RenderContext` implementations are guarded stubs carrying `// TODO(android-only)` and returning `nullptr` immediately (build-green, no host rendering). The existing `Surface::Create(w,h)` raster path is unchanged (FR-012).
- Unknown formats return `nullptr` — defined error state, never corrupt output.

## Flush / Swap Semantics

- `kCPU`: `Flush()` = write-back (unlock). Raster draws are immediate.
- `kGPU` buffer target: `Flush()` submits via `ctx->gr`; buffer must stay valid until submission completes.
- `kGPU` encoder-surface target: `SwapBuffers()` presents the composed frame to the encoder input queue (zero-copy); must be preceded by `MakeCurrent()` and a `gr->flush()`.

## Threading

One render thread owns the `RenderContext`. `MakeCurrent()` before any Skia GPU call; `SwapBuffers()` after flush. No concurrent use of `GrDirectContext`.
