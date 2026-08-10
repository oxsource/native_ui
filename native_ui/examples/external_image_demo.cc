// ExternalImage closed-loop demo (Android only, min API 29).
//
//   PNG -> RGBA -> AHardwareBuffer -> ExternalImage widget -> canvas hosted on the
//   MediaCodec encoder input surface (RenderContext, GLES/EGL) -> eglSwapBuffers ->
//   AMediaCodec H.264 -> AMediaMuxer -> <output>.mp4 (zero-copy, no CPU readback).
//
// On host builds the demo is a stub that exits non-zero (Android-only code path).

#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

#include "ahwb.h"
#include "android_media.h"
#include "canvas.h"
#include "external_image.h"
#include "hardware_buffer.h"
#include "image.h"
#include "render_context.h"
#include "surface.h"

#if defined(__ANDROID__)
#include <GLES3/gl3.h>
#include <android/hardware_buffer.h>

#include "SkCanvas.h"
#include "SkColorSpace.h"
#include "SkImageInfo.h"
#include "SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"
#endif

using namespace native::ui;

namespace {

#if defined(__ANDROID__)
constexpr int kFrameCount = 3;  // a few frames so the encoder emits a decodable keyframe

bool RenderOneFrame(const std::unique_ptr<ExternalImage>& ext,
                    const std::unique_ptr<Surface>& enc_surface, RenderContext* ctx,
                    AndroidMediaEncoder* encoder) {
  {
    Canvas canvas(*enc_surface);
    enc_surface->sk_canvas()->clear(SK_ColorDKGRAY);
    ext->Draw(canvas);
  }
  ctx->gr->flush();
  ctx->SwapBuffers();  // present -> encoder input queue (zero-copy)
  encoder->Poll();     // drain encoded output into the muxer
  return true;
}
#endif

}  // namespace

int main(int argc, char** argv) {
#if defined(__ANDROID__)
  const char* png_path = argc > 1 ? argv[1] : "/data/local/tmp/police.png";
  const char* mp4_path = argc > 2 ? argv[2] : "/data/local/tmp/external_image.mp4";

  // 1. Load the PNG and decode to RGBA.
  auto src = Image::FromFile(png_path);
  if (!src) {
    std::fprintf(stderr, "FAIL: cannot load %s\n", png_path);
    return 1;
  }
  const int w = src->width();
  const int h = src->height();
  if (w <= 0 || h <= 0) {
    std::fprintf(stderr, "FAIL: invalid image size %dx%d\n", w, h);
    return 1;
  }
  std::vector<uint8_t> rgba(static_cast<size_t>(w) * h * 4);
  auto info = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
  if (!src->sk_image()->readPixels(info, rgba.data(), static_cast<size_t>(w) * 4, 0, 0)) {
    std::fprintf(stderr, "FAIL: readPixels\n");
    return 1;
  }

  // 2. Allocate an AHardwareBuffer and write RGBA, injecting +16B/row padding (FR-002).
  AHardwareBuffer* ahwb = AHwb::AllocateRgba(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
  if (!ahwb) {
    std::fprintf(stderr, "FAIL: AllocateRgba\n");
    return 1;
  }
  const size_t padded_row_bytes = static_cast<size_t>(w) * 4 + 16;
  if (AHwb::WriteRgba(ahwb, rgba.data(), padded_row_bytes) != 0) {
    std::fprintf(stderr, "FAIL: WriteRgba\n");
    AHwb::Release(ahwb);
    return 1;
  }

  // 3. Wrap (non-owning, FR-011) and feed the widget.
  auto hb = HardwareBuffer::FromAHardwareBuffer(ahwb);
  auto ext = std::make_unique<ExternalImage>(hb);
  ext->SetBounds({0, 0, static_cast<float>(w), static_cast<float>(h)});

  // 4. Encoder + RenderContext hosted on the encoder input surface.
  auto encoder = AndroidMediaEncoder::Create(mp4_path, w, h);
  if (!encoder) {
    std::fprintf(stderr, "FAIL: AndroidMediaEncoder::Create\n");
    AHwb::Release(ahwb);
    return 1;
  }
  auto ctx = RenderContext::CreateFromMediaCodecInputSurface(encoder->input_window(), w, h);
  if (!ctx) {
    std::fprintf(stderr, "FAIL: RenderContext::CreateFromMediaCodecInputSurface\n");
    AHwb::Release(ahwb);
    return 1;
  }
  ctx->MakeCurrent();

  // 5. Wrap the encoder surface (default framebuffer, FBO 0) as the canvas target.
  GrGLFramebufferInfo fb_info{};
  fb_info.fFBOID = 0;
  fb_info.fFormat = GL_RGBA8;
  GrBackendRenderTarget rt = GrBackendRenderTargets::MakeGL(w, h, /*sampleCnt=*/0, /*stencil=*/8,
                                                            fb_info);
  if (!rt.isValid()) {
    std::fprintf(stderr, "FAIL: GrBackendRenderTargets::MakeGL\n");
    AHwb::Release(ahwb);
    return 1;
  }
  sk_sp<SkSurface> enc_surface = SkSurfaces::WrapBackendRenderTarget(
      ctx->gr, rt, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType, /*colorSpace=*/nullptr,
      /*surfaceProps=*/nullptr);
  if (!enc_surface) {
    std::fprintf(stderr, "FAIL: SkSurfaces::WrapBackendRenderTarget\n");
    AHwb::Release(ahwb);
    return 1;
  }
  auto enc_fw_surface = Surface::CreateFromSkSurface(std::move(enc_surface));
  if (!enc_fw_surface) {
    AHwb::Release(ahwb);
    return 1;
  }

  // 6. Compose the widget tree onto the encoder surface and present (zero-copy loop).
  for (int i = 0; i < kFrameCount; ++i) {
    RenderOneFrame(ext, enc_fw_surface, ctx.get(), encoder.get());
  }

  // 7. Finish (EOS + finalize MP4) and cleanup.
  encoder->Finish();
  AHwb::Release(ahwb);

  // Verify the output file was produced.
  std::FILE* f = std::fopen(mp4_path, "rb");
  if (!f) {
    std::fprintf(stderr, "FAIL: %s not written\n", mp4_path);
    return 1;
  }
  std::fseek(f, 0, SEEK_END);
  long size = std::ftell(f);
  std::fclose(f);
  std::printf("OK: %s written (%ld bytes), %dx%d\n", mp4_path, size, w, h);
  return size > 0 ? 0 : 1;
#else
  (void)argc;
  (void)argv;
  std::fprintf(stderr,
               "external_image_demo is Android-only (build with --config=android_arm64)\n");
  return 1;
#endif
}
