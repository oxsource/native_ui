# Contract: MediaCodec Integration (NDK native API)

**Scope**: How MediaCodec frames and surfaces connect to the framework's `ExternalImage`/`RenderContext`. **Min API 29 (Android 10)**. **GLES/EGL only.** MediaCodec is consumed through the **native NDK C API** — `<media/NdkMediaCodec.h>`, `<media/NdkMediaFormat.h>`, `<media/NdkMediaMuxer.h>` — **no JNI anywhere**. Link the example against `-lmediandk -landroid -lEGL -lGLESv3`.

## Decode Side (context for this feature, future)

The encoder-surface closed loop stands in for real decode in the PNG-driven demo. When real video is wired up later, it follows this contract — it requires **no new framework machinery**:

| Step | API (NDK) | Notes |
|------|-----|-------|
| Configure | `AMediaCodec_createDecoderByType(...)` + `AMediaCodec_configure(format, nullptr, nullptr, 0)` | No output surface → output arrives as an `Image` |
| Output | `AMediaCodec_getOutputImage(codec, index)` → `android.hardware.HardwareBuffer` | `getOutputImage` requires API 29+; min API 29 → unconditional |
| Hardware buffer | `HardwareBuffer::FromAHardwareBuffer(ahwb)` | Non-owning wrap |
| Into renderer | `Image::FromBuffer(hb, kGPU, ctx)` | `GrAHardwareBufferUtils` import → zero-copy texture |
| Widget | `ExternalImage(hb, ...)` | Draws the video texture |

**Constraints**: buffer must outlive the `Image` drawn from it (GPU path); release via `AHwb::Release` after the frame is no longer sampled. Thread: `AMediaCodec_getOutputImage` on the decode callback thread; the `Image`/buffer handoff must be synchronized with the render thread.

**Alternative (documented, NOT this feature)**: `AMediaCodec_configure(format, surface, ...)` with a `Surface(SurfaceTexture)` → frame lands in a `GL_TEXTURE_EXTERNAL_OES` texture; adopt via `GrGLTextureInfo` + `AdoptTextureFrom`. This is the future "video texture widget" path and must share the same EGL context as `ctx` to stay zero-copy.

## Encode Side (in scope — canvas hosted on the encoder surface)

| Step | API (NDK) | Notes |
|------|-----|-------|
| Encoder | `AMediaCodec_createEncoderByType("video/avc")` + `AMediaCodec_configure(...)` | H.264 baseline, size = frame size |
| Input surface | `AMediaCodec_createInputSurface(codec, &window)` | Returns `ANativeWindow*` — the surface MediaCodec provides; sole purpose is to receive GL-rendered frames (API 24+, min API 29 ✓) |
| EGL surface | `eglCreateWindowSurface(display, config, window)` | In `RenderContext::CreateFromMediaCodecInputSurface(ANativeWindow*)` |
| Skia target | `SkSurfaces::WrapBackendRenderTarget(ctx->gr, GrGLRenderTargetInfo{fFBOID=0, ...})` | Window surface = default framebuffer |
| Render | `ctx->MakeCurrent()` → draw widget tree → `ctx->gr->flush()` | Canvas draws `ExternalImage` texture + UI |
| Present | `ctx->SwapBuffers()` (`eglSwapBuffers`) | Encoder dequeues the presented buffer — **zero-copy** |
| Mux | `AMediaCodec_dequeueOutputBuffer` → `AMediaMuxer_addTrack` / `AMediaMuxer_writeSampleData` | Write `/tmp/external_image.mp4` |

**Zero-copy invariant**: the EGLSurface backing `AMediaCodec_createInputSurface()` and the Skia `GrDirectContext` must share one EGLContext (`RenderContext` enforces this). Buffer usage on the encoder input queue is `GPU_COLOR_OUTPUT`; the composed result is presented, never read back to CPU.

## Threading Model

- One render thread owns the `RenderContext` (create → MakeCurrent → draw → flush → SwapBuffers).
- Decode callback thread (future) hands buffers to the render thread; no concurrent `GrDirectContext` use.
- Encoder buffer dequeue runs on its own thread after each `SwapBuffers`.
- `AMediaCodec_getOutputImage` and `AHwb` locks must not overlap the render thread's GPU work.

## Error Handling

- `AMediaCodec_createInputSurface` / EGL creation failure → `RenderContext` factory returns `nullptr` → caller falls back to CPU backend (or aborts the encode path with a clear log).
- No exceptions; status codes / null returns only (`AMediaCodec` returns `AMEDIA_OK` or negative error codes).
