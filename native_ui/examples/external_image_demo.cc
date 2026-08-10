// ExternalImage closed-loop demo (Android only, min API 29).
//
//   PNG -> RGBA -> AHardwareBuffer -> ExternalImage widget -> canvas hosted on the
//   MediaCodec encoder input surface (RenderContext, GLES/EGL) -> eglSwapBuffers ->
//   AMediaCodec H.264 -> AMediaMuxer -> <output>.mp4 (zero-copy, no CPU readback).
//
//   --live[=seconds] mode: cycles a set of distinct AHardwareBuffers through the
//   widget at ~30 Hz for `seconds` (default 60), exercising the live-update path,
//   frame responsiveness, and bounded memory (FR-003/FR-004/FR-008).
//
// On host builds the demo is a stub that exits non-zero (Android-only code path).

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "ahwb.h"
#include "android_media.h"
#include "canvas.h"
#include "external_image.h"
#include "hardware_buffer.h"
#include "image.h"
#include "png_writer.h"
#include "render_context.h"
#include "surface.h"

#if defined(__ANDROID__)
#include <unistd.h>

#include <chrono>

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
constexpr int kFrameCount = 3;  // static mode: a few frames so the encoder emits a keyframe
constexpr int kLiveFps = 30;
constexpr int kLiveBuffers = 3;  // distinct buffers cycled by the live mode

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

// Resident set size (kB) read from /proc/self/status; -1 on failure.
int64_t ReadVmRssKb() {
  std::FILE* f = std::fopen("/proc/self/status", "r");
  if (!f) return -1;
  char line[256];
  int64_t rss = -1;
  while (std::fgets(line, sizeof(line), f)) {
    if (std::strncmp(line, "VmRSS:", 6) == 0) {
      rss = std::atoll(line + 6);
      break;
    }
  }
  std::fclose(f);
  return rss;
}

// Tinted copy of the RGBA source so live-mode frames are visibly distinct.
std::vector<uint8_t> MakeTinted(const std::vector<uint8_t>& rgba, int tint) {
  auto out = rgba;
  for (size_t i = 0; i + 3 < out.size(); i += 4) {
    if (tint == 1) {        // red-shift
      out[i] = static_cast<uint8_t>((out[i] * 5 + out[i + 1] + out[i + 2]) / 7);
    } else if (tint == 2) {  // green-shift
      out[i + 1] = static_cast<uint8_t>((out[i] + out[i + 1] * 5 + out[i + 2]) / 7);
    }
  }
  return out;
}
#endif

}  // namespace

int main(int argc, char** argv) {
#if defined(__ANDROID__)
  const char* png_path = "/data/local/tmp/police.png";
  const char* mp4_path = "/data/local/tmp/external_image.mp4";
  bool live = false;
  int live_seconds = 60;
  int pos = 0;
  for (int i = 1; i < argc; ++i) {
    if (std::strncmp(argv[i], "--live", 6) == 0) {
      live = true;
      if (argv[i][6] == '=') live_seconds = std::atoi(argv[i] + 7);
      if (live_seconds <= 0) live_seconds = 60;
    } else if (pos == 0) {
      png_path = argv[i];
      ++pos;
    } else if (pos == 1) {
      mp4_path = argv[i];
      ++pos;
    }
  }

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
  const size_t padded_row_bytes = static_cast<size_t>(w) * 4 + 16;  // FR-002 stride padding

  // 2. Allocate AHardwareBuffer(s) and write RGBA. Static mode: one source buffer.
  //    Live mode: kLiveBuffers distinct buffers cycled at 30 Hz.
  const int buffer_count = live ? kLiveBuffers : 1;
  std::vector<AHardwareBuffer*> ahwb_list(buffer_count, nullptr);
  std::vector<HardwareBuffer> hb_list(buffer_count);
  for (int i = 0; i < buffer_count; ++i) {
    std::vector<uint8_t> pixels = live ? MakeTinted(rgba, i) : rgba;
    ahwb_list[i] = AHwb::AllocateRgba(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
    if (!ahwb_list[i]) {
      std::fprintf(stderr, "FAIL: AllocateRgba[%d]\n", i);
      for (int j = 0; j < i; ++j) AHwb::Release(ahwb_list[j]);
      return 1;
    }
    if (AHwb::WriteRgba(ahwb_list[i], pixels.data(), padded_row_bytes) != 0) {
      std::fprintf(stderr, "FAIL: WriteRgba[%d]\n", i);
      for (int j = 0; j <= i; ++j) AHwb::Release(ahwb_list[j]);
      return 1;
    }
    hb_list[i] = HardwareBuffer::FromAHardwareBuffer(ahwb_list[i]);
  }

  // 3. Wrap (non-owning, FR-011) and feed the widget.
  auto ext = std::make_unique<ExternalImage>(hb_list[0]);
  ext->SetBounds({0, 0, static_cast<float>(w), static_cast<float>(h)});

  // 4. Encoder + RenderContext hosted on the encoder input surface.
  auto encoder = AndroidMediaEncoder::Create(mp4_path, w, h);
  if (!encoder) {
    std::fprintf(stderr, "FAIL: AndroidMediaEncoder::Create\n");
    for (auto* b : ahwb_list) AHwb::Release(b);
    return 1;
  }
  auto ctx = RenderContext::CreateFromMediaCodecInputSurface(encoder->input_window(), w, h);
  if (!ctx) {
    std::fprintf(stderr, "FAIL: RenderContext::CreateFromMediaCodecInputSurface\n");
    for (auto* b : ahwb_list) AHwb::Release(b);
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
    for (auto* b : ahwb_list) AHwb::Release(b);
    return 1;
  }
  sk_sp<SkSurface> enc_surface = SkSurfaces::WrapBackendRenderTarget(
      ctx->gr, rt, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType, /*colorSpace=*/nullptr,
      /*surfaceProps=*/nullptr);
  if (!enc_surface) {
    std::fprintf(stderr, "FAIL: SkSurfaces::WrapBackendRenderTarget\n");
    for (auto* b : ahwb_list) AHwb::Release(b);
    return 1;
  }
  auto enc_fw_surface = Surface::CreateFromSkSurface(std::move(enc_surface));
  if (!enc_fw_surface) {
    for (auto* b : ahwb_list) AHwb::Release(b);
    return 1;
  }

  if (!live) {
    // 6. Static mode: compose a few frames and present (zero-copy loop).
    for (int i = 0; i < kFrameCount; ++i) {
      RenderOneFrame(ext, enc_fw_surface, ctx.get(), encoder.get());
    }
  } else {
    // 6'. Live mode: cycle buffers at ~30 Hz for live_seconds, tracking frame times
    //     and resident memory (FR-003/FR-004, SC-002/003/004).
    const int live_frames = live_seconds * kLiveFps;
    const int64_t rss_before = ReadVmRssKb();
    const auto start = std::chrono::steady_clock::now();
    double sum_us = 0;
    int64_t max_us = 0;
    std::printf("[live] cycling %d buffers at %d Hz for %d s (%d frames)\n", kLiveBuffers,
                kLiveFps, live_seconds, live_frames);
    for (int i = 0; i < live_frames; ++i) {
      const auto f0 = std::chrono::steady_clock::now();
      ext->SetBuffer(hb_list[i % kLiveBuffers]);
      RenderOneFrame(ext, enc_fw_surface, ctx.get(), encoder.get());
      const auto f1 = std::chrono::steady_clock::now();
      const int64_t us = std::chrono::duration_cast<std::chrono::microseconds>(f1 - f0).count();
      sum_us += static_cast<double>(us);
      if (us > max_us) max_us = us;
      if ((i + 1) % 300 == 0) {
        std::printf("[live] frame %d/%d  avg %.2f ms  max %.2f ms\n", i + 1, live_frames,
                    sum_us / (i + 1) / 1000.0, static_cast<double>(max_us) / 1000.0);
      }
      // Throttle to the target frame rate if we're running ahead.
      const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(f1 - start).count();
      const int64_t target_us = static_cast<int64_t>(i + 1) * 1000000 / kLiveFps;
      if (target_us > elapsed) {
        usleep(static_cast<useconds_t>(target_us - elapsed));
      }
    }
    const auto end = std::chrono::steady_clock::now();
    const double total_s =
        std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    const int64_t rss_after = ReadVmRssKb();
    std::printf("[live] done %d frames in %.1f s (avg %.2f ms, max %.2f ms)\n", live_frames,
                total_s, sum_us / live_frames / 1000.0, static_cast<double>(max_us) / 1000.0);
    if (rss_before > 0 && rss_after > 0) {
      std::printf("[live] VmRSS %lld -> %lld kB (%+lld kB)\n", static_cast<long long>(rss_before),
                  static_cast<long long>(rss_after),
                  static_cast<long long>(rss_after - rss_before));
    }
  }

  // 6b. Diagnostic export (FR-010, SC-006): CPU snapshot of the displayed frame and a
  //     PNG dump of the source buffer for pixel verification.
  {
    auto cpu_surface = Surface::Create(w, h);
    if (cpu_surface) {
      Canvas canvas(*cpu_surface);
      cpu_surface->sk_canvas()->clear(SK_ColorDKGRAY);
      ext->Draw(canvas);
      cpu_surface->Flush();
      const char* cpu_png = "/data/local/tmp/external_image_cpu.png";
      if (PngWriter::Write(cpu_surface->sk_surface(), cpu_png)) {
        std::printf("OK: %s written\n", cpu_png);
      } else {
        std::fprintf(stderr, "FAIL: PngWriter::Write %s\n", cpu_png);
      }
    }
    const char* source_png = "/data/local/tmp/source_buffer.png";
    if (AHwb::DumpPng(ahwb_list[0], source_png) > 0) {
      std::printf("OK: %s written\n", source_png);
    }
  }

  // 7. Finish (EOS + finalize MP4) and cleanup.
  encoder->Finish();
  for (auto* b : ahwb_list) AHwb::Release(b);

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
