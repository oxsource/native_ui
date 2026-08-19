#pragma once

#include <memory>

#include "src/framework/core/color.h"

class GrDirectContext;

namespace native::ui {

// GPU/EGL context bundle: the Skia GPU context (gr) is built on a shared EGL
// context that also hosts the encoder-input-surface render target
// (single-context rule — see contracts/render-backend.md). Android-only; the
// factory returns nullptr on host builds (guarded stub).
struct RenderContext {
  GrDirectContext* gr = nullptr;  // Skia GPU context on the shared EGL context
  void* display = nullptr;        // EGLDisplay
  void* context = nullptr;        // EGLContext (GLES 3.x)
  void* surface = nullptr;        // EGLSurface — eglCreateWindowSurface(AMediaCodec_createInputSurface())
  int width = 0;                  // configured frame size
  int height = 0;
  ColorSpace color_space = ColorSpace::kSRGB;  // render-target color space

  // __ANDROID__ only; nullptr on failure/host. `surface` is the ANativeWindow* (from
  // AMediaCodec_createInputSurface, kept opaque here to avoid the EGL typedef dance on host).
  static std::unique_ptr<RenderContext> CreateFromNativeWindow(
      void* surface, int width, int height,
      ColorSpace color_space = ColorSpace::kSRGB);

  void MakeCurrent();   // eglMakeCurrent + gr context current
  // Set the presentation timestamp (ns) of the next buffer presented on the
  // encoder input surface, via eglPresentationTimeANDROID. Without this the
  // MediaCodec encoder receives frames with a stale/system timestamp and the
  // produced packets have non-monotonic dts. Returns false when the extension
  // is unavailable (then the system time is used). host: no-op (returns false).
  bool SetPresentationTimeNs(int64_t timestamp_ns);
  void SwapBuffers();   // eglSwapBuffers -> presents frame to encoder

  ~RenderContext();

private:
  // Owns the GrDirectContext (sk_sp<GrDirectContext>) as an opaque pointer so the
  // header compiles on host builds where the GPU type is unavailable. Destroyed in
  // ~RenderContext before the EGL handles. Unused on host (factory returns nullptr).
  [[maybe_unused]] void* gr_owner_ = nullptr;
};

}  // namespace native::ui
