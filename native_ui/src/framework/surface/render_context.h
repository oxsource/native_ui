#pragma once

#include <memory>

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

  // __ANDROID__ only; nullptr on failure/host. `surface` is the ANativeWindow* from
  // AMediaCodec_createInputSurface (kept opaque here to avoid the EGL typedef dance on host).
  static std::unique_ptr<RenderContext> CreateFromMediaCodecInputSurface(
      void* surface, int width, int height);

  void MakeCurrent();   // eglMakeCurrent + gr context current
  void SwapBuffers();   // eglSwapBuffers -> presents frame to encoder

  ~RenderContext();

private:
  // Owns the GrDirectContext (sk_sp<GrDirectContext>) as an opaque pointer so the
  // header compiles on host builds where the GPU type is unavailable. Destroyed in
  // ~RenderContext before the EGL handles. Unused on host (factory returns nullptr).
  [[maybe_unused]] void* gr_owner_ = nullptr;
};

}  // namespace native::ui
