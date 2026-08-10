# Data / Entity Model: Android AHardwareBuffer ExternalImage

**Date**: 2026-08-03

## Entity: HardwareBuffer (cross-platform wrapper)

| Field | Type | Description |
|-------|------|-------------|
| `kind` | `Memory` / `AHardwareBuffer` / `Invalid` | Backing storage of the buffer |
| `handle` | `void*` (or AHardwareBuffer*) | Platform handle; AHardwareBuffer on Android |
| `pixels` | `void*` (Memory only) | Host CPU pixel pointer |
| `width` / `height` | `int` | Cached from describe (AHwb) or constructor (Memory) |
| `row_bytes` | `size_t` | Stride in bytes; may exceed `width * 4` (FR-002) |
| `format` | `int` | Pixel format id (0 = unknown) |
| `valid` | `bool` | Derived from non-null handle + positive dims |

**Ownership**: non-owning. The producer retains the buffer; the framework copies whatever it renders (FR-011). Factories: `FromAHardwareBuffer(void*)` (Android), `FromMemory(void*, row_bytes, w, h)` (all platforms — data-only, reserved for host wrapper tests / future non-Android use).

**Equality**: `operator==` compares the underlying handle (used by `ExternalImage` to skip redundant conversion).

## Entity: AHwb (Android utility layer)

| Function | Input → Output | Lifecycle |
|----------|----------------|-----------|
| `Describe` | `AHardwareBuffer*` → `{w,h,stride,format}` | read-only |
| `Lock` / `Unlock` | `AHardwareBuffer*`, usage, `&data` | RAII pair |
| `Pixels` | buffer, usage, `fn(void*)` | lock → fn → unlock (guaranteed) |
| `AllocateRGBA` | `(w,h)` → `AHardwareBuffer*` | allocate R8G8B8A8_UNORM |
| `WriteRGBA` | buffer, `src`, `src_row_bytes` | lock(WRITE) → per-row copy honoring dst stride → unlock |
| `Release` | `AHardwareBuffer*` | free |
| `ToCpuImage` | buffer → `sk_sp<SkImage>` | describe → lock(READ) → owned copy → unlock |
| `ToGpuImage` | buffer, `GrDirectContext*` → `sk_sp<SkImage>` | backend-texture wrap (zero-copy) |
| `DumpPng` | buffer, path | read → PNG export |

All Android-only (compiled as error-code stubs on host).

## Entity: Image (render resource from a buffer)

| Attribute | Value |
|-----------|-------|
| Produced by | `Image::FromBuffer(buffer, backend, ctx)` |
| CPU backend | Owned `SkBitmap`/`SkImage` copied from buffer memory (stride honored) |
| GPU backend | Zero-copy `SkImage` wrapping the buffer texture (`GrAHardwareBufferUtils`, GLES/EGL) |
| Lifetime | Owned by the `Image`; independent of producer's buffer lifetime (CPU path) |
| Invalid/unsupported | `nullptr` (defined error state, FR-006) |

## Entity: ExternalImage (widget)

| Attribute | Value |
|-----------|-------|
| `buffer_` | Currently bound `HardwareBuffer` (non-owning) |
| `image_` | `std::unique_ptr<Image>` rebuilt only when buffer handle changes |
| `watched_prop_` | Optional `Property<HardwareBuffer>*` for live updates |
| Draw | Rebuilds `image_` from watched value on handle change; draws to `Canvas` via `DrawImage` |

## Entity: Surface (render target)

| Attribute | Value |
|-----------|-------|
| `backend` | `RenderBackend::kCPU` (Android fallback) / `kGPU` (GLES/EGL, API 29+); host = guarded stub (`nullptr`) |
| CPU | `SkSurfaces::WrapPixels` over locked buffer memory; buffer held locked for surface lifetime; unlock on `Flush`/destruct |
| GPU (buffer target) | `SkSurfaces::WrapBackendTexture` over AHardwareBuffer backend texture; `Flush` submits |
| GPU (encoder-surface target) | `SkSurfaces::WrapBackendRenderTarget` over `ctx->surface` (EGLSurface from `AMediaCodec_createInputSurface()` → `ANativeWindow*`); `Flush` → `SwapBuffers` presents to encoder |
| `CreateFromBuffer` | `(buffer, backend, ctx)`; GPU without `ctx` → CPU fallback → `nullptr` |

## Entity: RenderContext (GPU/EGL bundle)

| Attribute | Value |
|-----------|-------|
| `gr` | `GrDirectContext*` on the shared EGL context (Skia GPU entry) |
| `display` / `context` / `surface` | EGLDisplay / EGLContext (GLES 3.x) / EGLSurface |
| Factory | `CreateFromMediaCodecInputSurface(ANativeWindow* surface, w, h)` — `eglCreateWindowSurface(AMediaCodec_createInputSurface())` → `GrDirectContext::MakeGL` |
| Methods | `MakeCurrent()`, `SwapBuffers()` (present to encoder) |
| Platform | `__ANDROID__` only; host = guarded stub returning `nullptr` |
| Invariant | one EGLContext hosts both imports and the render target (zero-copy) |

## Entity: MediaCodec integration (encode egress in scope; decode ingress future)

| Step | Entity/API |
|------|-----------|
| Encode | `AMediaCodec_createInputSurface(&window)` (`ANativeWindow*`) → `RenderContext::CreateFromMediaCodecInputSurface` → canvas → `SwapBuffers()` → `AMediaCodec_dequeueOutputBuffer` → `AMediaMuxer` → MP4 (NDK native API, no JNI) |
| Decode (future) | `AMediaCodec_getOutputImage` → `getHardwareBuffer()` → `FromAHardwareBuffer` → `FromBuffer(kGPU, ctx)` → `ExternalImage` |

## State Transitions

```
Buffer lifecycle (producer side):
  allocate → fill (lock+write+unlock) → wrap in HardwareBuffer
  → [framework consumes: copy (CPU) | texture wrap (GPU)]
  → release (producer)

Widget state:
  bind(buffer) → FromBuffer → image_ (nullptr if unsupported)
  property update (handle changed) → rebuild image_ → RequestRedraw
  Draw → DrawImage(image_, bounds)

GPU encode loop (closed loop):
  AMediaCodec_createInputSurface(codec, &window)            // ANativeWindow*
  RenderContext::CreateFromMediaCodecInputSurface(window, w, h)
  → MakeCurrent → draw ExternalImage + UI (canvas on encoder surface)
  → gr->flush → SwapBuffers (present, zero-copy)
  → AMediaCodec_dequeueOutputBuffer → AMediaMuxer_writeSampleData
  → EOS → finalize → /tmp/external_image.mp4
```

## Validation Rules

| Rule | Entity | Description |
|------|--------|-------------|
| Stride padding ignored (FR-002) | WriteRGBA / ToCpuImage / WrapPixels | visible width only, `row_bytes` honored |
| Invalid/empty buffer → no render, no crash (FR-005) | ExternalImage | `Draw` no-ops when `image_` is null |
| Unsupported format → defined error, no corrupt output (FR-006) | FromBuffer | returns `nullptr` |
| Producer release safe (FR-011) | Image (CPU) | owned copy, not a borrowed pointer |
| Bounded memory over 10k updates (FR-004) | ExternalImage + Image | owned images replaced, never accumulated |
| No host/CI regression (FR-012) | Surface/Image | `Create(w,h)` raster path unchanged; Android code is guarded stubs on host |
