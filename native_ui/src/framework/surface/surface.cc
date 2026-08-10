#include "surface.h"

#include "SkCanvas.h"
#include "SkImageInfo.h"
#include "SkSurface.h"

#if defined(__ANDROID__)
#include <android/hardware_buffer.h>

#include "SkColorSpace.h"
#include "ahwb.h"
#include "render_context.h"
#include "include/android/GrAHardwareBufferUtils.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#endif

namespace native::ui {

class SurfaceImpl {
public:
  ~SurfaceImpl() {
#if defined(__ANDROID__)
    // Unlock a CPU buffer that was held locked for the surface's lifetime.
    if (locked_buffer) {
      AHwb::Unlock(locked_buffer);
      locked_buffer = nullptr;
    }
#endif
  }

  sk_sp<SkSurface> sk_surface;
#if defined(__ANDROID__)
  AHardwareBuffer* locked_buffer = nullptr;  // CPU backend: kept locked until Flush/destruct
  GrDirectContext* gr = nullptr;             // GPU backend: flush via the context (SkSurface has no flush())
#endif
};

Surface::Surface(SurfaceImpl* impl) : impl_(impl) {}

Surface::~Surface() { delete impl_; }

std::unique_ptr<Surface> Surface::Create(int width, int height) {
  auto* impl = new SurfaceImpl();
  auto info = SkImageInfo::MakeN32Premul(width, height);
  impl->sk_surface = SkSurfaces::Raster(info);
  if (!impl->sk_surface) {
    delete impl;
    return nullptr;
  }
  return std::unique_ptr<Surface>(new Surface(impl));
}

std::unique_ptr<Surface> Surface::CreateFromSkSurface(sk_sp<SkSurface> sk_surface) {
  if (!sk_surface) return nullptr;
  auto* impl = new SurfaceImpl();
  impl->sk_surface = std::move(sk_surface);
  return std::unique_ptr<Surface>(new Surface(impl));
}

std::unique_ptr<Surface> Surface::CreateFromBuffer(HardwareBuffer buffer,
                                                   RenderBackend backend,
                                                   RenderContext* ctx) {
  if (!buffer.IsValid()) return nullptr;

#if defined(__ANDROID__)
  if (buffer.kind() != HardwareBuffer::Kind::kAHardwareBuffer) {
    // Memory kind has no Android render path (reserved; TODO(android-only)).
    return nullptr;
  }
  AHardwareBuffer* ahwb = static_cast<AHardwareBuffer*>(buffer.ahardwarebuffer());
  if (!ahwb) return nullptr;

  RenderBackend effective = backend;
  if (effective == RenderBackend::kGPU && (!ctx || !ctx->gr)) {
    effective = RenderBackend::kCPU;  // fall back to CPU per contracts/render-backend.md
  }

  uint32_t w = 0, h = 0, stride = 0;
  int format = 0;
  if (!AHwb::Describe(ahwb, w, h, stride, format)) return nullptr;
  if (format != AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM) return nullptr;  // FR-006

  if (effective == RenderBackend::kCPU) {
    // Zero-copy raster surface drawing directly into the locked buffer memory.
    // The buffer stays locked for the surface's lifetime; Flush()/~Surface unlock (write-back).
    void* data = nullptr;
    if (AHwb::Lock(ahwb, AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN,
                   &data) != 0) {
      return nullptr;
    }
    auto* impl = new SurfaceImpl();
    impl->locked_buffer = ahwb;
    auto info = SkImageInfo::Make(static_cast<int>(w), static_cast<int>(h), kRGBA_8888_SkColorType,
                                  kPremul_SkAlphaType);
    impl->sk_surface = SkSurfaces::WrapPixels(info, data, static_cast<size_t>(stride) * 4);
    if (!impl->sk_surface) {
      AHwb::Unlock(ahwb);
      delete impl;
      return nullptr;
    }
    return std::unique_ptr<Surface>(new Surface(impl));
  }

  // GPU backend (buffer target): zero-copy backend-texture wrap.
  GrAHardwareBufferUtils::DeleteImageProc delete_proc = nullptr;
  GrAHardwareBufferUtils::UpdateImageProc update_proc = nullptr;
  GrAHardwareBufferUtils::TexImageCtx image_ctx = nullptr;
  GrBackendFormat backend_format = GrAHardwareBufferUtils::GetGLBackendFormat(
      ctx->gr, static_cast<uint32_t>(format), /*requireKnownFormat=*/true);
  GrBackendTexture tex = GrAHardwareBufferUtils::MakeGLBackendTexture(
      ctx->gr, ahwb, static_cast<int>(w), static_cast<int>(h), &delete_proc, &update_proc,
      &image_ctx, /*isProtectedContent=*/false, backend_format, /*isRenderable=*/true);
  if (!tex.isValid() || !delete_proc) return nullptr;

  auto* impl = new SurfaceImpl();
  impl->gr = ctx->gr;
  impl->sk_surface = SkSurfaces::WrapBackendTexture(
      ctx->gr, tex, kTopLeft_GrSurfaceOrigin, /*sampleCnt=*/1, kRGBA_8888_SkColorType,
      /*colorSpace=*/nullptr, /*surfaceProps=*/nullptr, delete_proc, image_ctx);
  if (!impl->sk_surface) {
    delete impl;
    return nullptr;
  }
  return std::unique_ptr<Surface>(new Surface(impl));
#else
  (void)backend;
  (void)ctx;
  // TODO(android-only): host builds stub the buffer-backed surface.
  return nullptr;
#endif
}

SkCanvas* Surface::sk_canvas() const {
  return impl_->sk_surface->getCanvas();
}

SkSurface* Surface::sk_surface() const {
  return impl_ ? impl_->sk_surface.get() : nullptr;
}

void Surface::Flush() {
#if defined(__ANDROID__)
  if (impl_) {
    if (impl_->locked_buffer) {
      AHwb::Unlock(impl_->locked_buffer);  // write-back (CPU backend)
      impl_->locked_buffer = nullptr;
    }
    if (impl_->gr && impl_->sk_surface) {
      impl_->gr->flush(impl_->sk_surface.get());  // submit GPU work (SkSurface has no flush())
    }
  }
#else
  // Raster surfaces do not require flushing.
#endif
}

int Surface::width() const {
  return impl_ ? impl_->sk_surface->width() : 0;
}

int Surface::height() const {
  return impl_ ? impl_->sk_surface->height() : 0;
}

}  // namespace native::ui
