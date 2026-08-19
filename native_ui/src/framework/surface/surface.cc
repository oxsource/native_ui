#include "src/framework/surface/surface.h"

#include <cstdio>
#include <vector>

#include "SkCanvas.h"
#include "SkColorSpace.h"
#include "SkImageInfo.h"
#include "SkSurface.h"
#include "stb_image_write.h"

#if defined(__ANDROID__)
#include <GLES3/gl3.h>
#include <android/hardware_buffer.h>

#include "src/framework/surface/ahwb.h"
#include "src/framework/surface/render_context.h"
#include "include/android/GrAHardwareBufferUtils.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"
#endif

namespace native::ui {

namespace {

// Maps the framework color space to the renderer (implementation detail).
sk_sp<SkColorSpace> ToSkColorSpace(ColorSpace cs) {
  return cs == ColorSpace::kLinearSRGB ? SkColorSpace::MakeSRGBLinear()
                                       : SkColorSpace::MakeSRGB();
}

}  // namespace

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

std::unique_ptr<Surface> Surface::Create(int width, int height, ColorSpace color_space) {
  auto* impl = new SurfaceImpl();
  auto info = SkImageInfo::MakeN32Premul(width, height).makeColorSpace(ToSkColorSpace(color_space));
  impl->sk_surface = SkSurfaces::Raster(info);
  if (!impl->sk_surface) {
    delete impl;
    return nullptr;
  }
  return std::unique_ptr<Surface>(new Surface(impl));
}

std::unique_ptr<Surface> Surface::Create(RenderContext* ctx) {
#if defined(__ANDROID__)
  if (!ctx || !ctx->gr || ctx->width <= 0 || ctx->height <= 0) return nullptr;
  ctx->MakeCurrent();
  // Encoder input surface = default framebuffer (FBO 0); GL origin is bottom-left.
  GrGLFramebufferInfo fb_info{};
  fb_info.fFBOID = 0;
  fb_info.fFormat = GL_RGBA8;
  GrBackendRenderTarget rt = GrBackendRenderTargets::MakeGL(ctx->width, ctx->height,
                                                            /*sampleCnt=*/0, /*stencil=*/8,
                                                            fb_info);
  if (!rt.isValid()) return nullptr;
  sk_sp<SkSurface> sk_surface = SkSurfaces::WrapBackendRenderTarget(
      ctx->gr, rt, kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
      ToSkColorSpace(ctx->color_space), /*surfaceProps=*/nullptr);
  if (!sk_surface) return nullptr;

  auto* impl = new SurfaceImpl();
  impl->gr = ctx->gr;
  impl->sk_surface = std::move(sk_surface);
  return std::unique_ptr<Surface>(new Surface(impl));
#else
  (void)ctx;
  // TODO(android-only): host builds stub the context-backed surface.
  return nullptr;
#endif
}

PixelBuffer Surface::Allocate(int width, int height, PixelFormat format) {
  PixelBuffer pb;
  if (width <= 0 || height <= 0) return pb;
  if (format != PixelFormat::kRGBA && format != PixelFormat::kBGRA) return pb;
  pb.width = width;
  pb.height = height;
  pb.format = format;
  pb.row_bytes = 0;  // tightly packed (width * 4)
  pb.data.assign(static_cast<size_t>(width) * static_cast<size_t>(height) * 4,
                 0);
  return pb;
}

std::unique_ptr<Surface> Surface::CreateFromPixels(PixelBuffer& pb) {
  if (pb.empty()) return nullptr;

  void* pixels = pb.data.data();  // WrapPixels takes a writable void*
  const int width = pb.width;
  const int height = pb.height;
  const PixelFormat format = pb.format;
  size_t row_bytes = pb.row_bytes;

  SkColorType ct;
  switch (format) {
    case PixelFormat::kRGBA:
      ct = kRGBA_8888_SkColorType;
      break;
    case PixelFormat::kBGRA:
      ct = kBGRA_8888_SkColorType;
      break;
    default:
      return nullptr;
  }
  if (row_bytes == 0) row_bytes = static_cast<size_t>(width) * 4;

  // WrapPixels is zero-copy: the raster surface points at the caller's buffer
  // and does not own it. The caller must keep the buffer alive while the
  // surface is used. sRGB, premultiplied alpha (matches Image decode output).
  auto info = SkImageInfo::Make(width, height, ct, kPremul_SkAlphaType,
                                SkColorSpace::MakeSRGB());
  auto* impl = new SurfaceImpl();
  impl->sk_surface =
      SkSurfaces::WrapPixels(info, pixels, row_bytes);
  if (!impl->sk_surface) {
    delete impl;
    return nullptr;
  }
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
      ToSkColorSpace(ctx->color_space), /*surfaceProps=*/nullptr, delete_proc, image_ctx);
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

bool Surface::Dump(const char* path) const {
  if (!impl_ || !impl_->sk_surface || !path) return false;
  SkPixmap pixmap;
  if (!impl_->sk_surface->peekPixels(&pixmap)) {
    std::fprintf(stderr, "FAIL: Dump peekPixels failed\n");
    return false;
  }
  int w = pixmap.width(), h = pixmap.height();
  std::vector<uint8_t> rgb(static_cast<size_t>(w) * h * 3);
  const auto* src = static_cast<const uint8_t*>(pixmap.addr());
  for (int i = 0; i < w * h; i++) {
    rgb[i * 3 + 0] = src[i * 4 + 0];
    rgb[i * 3 + 1] = src[i * 4 + 1];
    rgb[i * 3 + 2] = src[i * 4 + 2];
  }
  if (!stbi_write_png(path, w, h, 3, rgb.data(), 0)) {
    std::fprintf(stderr, "FAIL: Dump stbi_write_png\n");
    return false;
  }
  return true;
}

void* Surface::Handle() const {
  return impl_ ? static_cast<void*>(impl_->sk_surface.get()) : nullptr;
}

}  // namespace native::ui
