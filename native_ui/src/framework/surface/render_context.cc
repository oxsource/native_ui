#include "src/framework/surface/render_context.h"

#if defined(__ANDROID__)
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/native_window.h>

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#endif

namespace native::ui {

#if defined(__ANDROID__)

namespace {

// RGBA8888 window surface, GLES 3.x renderable (matches R8G8B8A8_UNORM buffers).
const EGLint kWindowAttribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_NONE,
};

const EGLint kContextAttribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 3,
    EGL_NONE,
};

}  // namespace

std::unique_ptr<RenderContext> RenderContext::CreateFromNativeWindow(void* surface,
                                                                               int width,
                                                                               int height,
                                                                               ColorSpace cs) {
  ANativeWindow* native_window = static_cast<ANativeWindow*>(surface);
  if (!native_window || width <= 0 || height <= 0) return nullptr;

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (display == EGL_NO_DISPLAY) return nullptr;
  if (eglInitialize(display, nullptr, nullptr) != EGL_TRUE) return nullptr;

  EGLConfig config = nullptr;
  EGLint num_configs = 0;
  if (eglChooseConfig(display, kWindowAttribs, &config, 1, &num_configs) != EGL_TRUE ||
      num_configs < 1) {
    eglTerminate(display);
    return nullptr;
  }

  EGLSurface egl_surface = eglCreateWindowSurface(display, config, native_window, nullptr);
  if (egl_surface == EGL_NO_SURFACE) {
    eglTerminate(display);
    return nullptr;
  }

  EGLContext egl_context = eglCreateContext(display, config, EGL_NO_CONTEXT, kContextAttribs);
  if (egl_context == EGL_NO_CONTEXT) {
    eglDestroySurface(display, egl_surface);
    eglTerminate(display);
    return nullptr;
  }

  if (eglMakeCurrent(display, egl_surface, egl_surface, egl_context) != EGL_TRUE) {
    eglDestroyContext(display, egl_context);
    eglDestroySurface(display, egl_surface);
    eglTerminate(display);
    return nullptr;
  }

  auto gr_interface = GrGLMakeNativeInterface();
  auto gr = GrDirectContexts::MakeGL(gr_interface);
  if (!gr) {
    eglDestroyContext(display, egl_context);
    eglDestroySurface(display, egl_surface);
    eglTerminate(display);
    return nullptr;
  }

  auto ctx = std::unique_ptr<RenderContext>(new RenderContext());
  ctx->gr_owner_ = new sk_sp<GrDirectContext>(std::move(gr));
  ctx->gr = static_cast<sk_sp<GrDirectContext>*>(ctx->gr_owner_)->get();
  ctx->display = display;
  ctx->context = egl_context;
  ctx->surface = egl_surface;
  ctx->width = width;
  ctx->height = height;
  ctx->color_space = cs;
  return ctx;
}

void RenderContext::MakeCurrent() {
  if (display && context && surface) {
    eglMakeCurrent(static_cast<EGLDisplay>(display), static_cast<EGLSurface>(surface),
                   static_cast<EGLSurface>(surface), static_cast<EGLContext>(context));
  }
}

void RenderContext::SwapBuffers() {
  if (display && surface) {
    eglSwapBuffers(static_cast<EGLDisplay>(display), static_cast<EGLSurface>(surface));
  }
}

RenderContext::~RenderContext() {
  // Release GPU resources first (they depend on the EGL context), then teardown EGL.
  delete static_cast<sk_sp<GrDirectContext>*>(gr_owner_);
  gr_owner_ = nullptr;
  gr = nullptr;
  if (display && context) {
    eglMakeCurrent(static_cast<EGLDisplay>(display), EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);
    eglDestroyContext(static_cast<EGLDisplay>(display), static_cast<EGLContext>(context));
  }
  if (display && surface) {
    eglDestroySurface(static_cast<EGLDisplay>(display), static_cast<EGLSurface>(surface));
  }
  if (display) {
    eglTerminate(static_cast<EGLDisplay>(display));
  }
}

#else  // !defined(__ANDROID__)

std::unique_ptr<RenderContext> RenderContext::CreateFromNativeWindow(void*, int, int,
                                                                              ColorSpace) {
  // TODO(android-only): host builds stub the GPU context.
  return nullptr;
}
void RenderContext::MakeCurrent() {}
void RenderContext::SwapBuffers() {}
RenderContext::~RenderContext() {}

#endif  // defined(__ANDROID__)

}  // namespace native::ui
