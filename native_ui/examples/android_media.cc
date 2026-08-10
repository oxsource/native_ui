#include "android_media.h"

#include <cerrno>
#include <cstdio>
#include <chrono>
#include <cstring>

#if defined(__ANDROID__)
#include <fcntl.h>
#include <unistd.h>

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <media/NdkMediaMuxer.h>
#endif

namespace native::ui {

#if defined(__ANDROID__)

namespace {

// Drain one output buffer into the muxer. Handles the OUTPUT_FORMAT_CHANGED info code.
bool DrainOutput(AMediaCodec* codec, AMediaMuxer* muxer, int& track_index, bool& started,
                 ssize_t status, const AMediaCodecBufferInfo& info) {
  if (status >= 0) {
    if (!started) return true;  // format must arrive first (or the info-code path below)
    size_t buf_size = 0;
    uint8_t* buf = AMediaCodec_getOutputBuffer(codec, status, &buf_size);
    if (buf && info.size > 0) {
      AMediaMuxer_writeSampleData(muxer, track_index, buf, &info);
    }
    AMediaCodec_releaseOutputBuffer(codec, status, /*render=*/false);
    return true;
  }
  if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
    AMediaFormat* out_format = AMediaCodec_getOutputFormat(codec);
    if (out_format) {
      track_index = AMediaMuxer_addTrack(muxer, out_format);
      AMediaFormat_delete(out_format);
      if (track_index >= 0) {
        AMediaMuxer_start(muxer);
        started = true;
      }
    }
    return true;
  }
  return false;  // AMEDIACODEC_INFO_TRY_AGAIN_LATER / END_OF_STREAM
}

}  // namespace

std::unique_ptr<AndroidMediaEncoder> AndroidMediaEncoder::Create(const char* path, int width,
                                                                int height) {
  if (!path || width <= 0 || height <= 0) {
    std::fprintf(stderr, "encoder: invalid args\n");
    return nullptr;
  }

  AMediaCodec* codec = AMediaCodec_createEncoderByType("video/avc");
  if (!codec) {
    std::fprintf(stderr, "encoder: AMediaCodec_createEncoderByType(video/avc) failed\n");
    return nullptr;
  }

  AMediaFormat* format = AMediaFormat_new();
  AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
  AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, width);
  AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, height);
  AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_BIT_RATE, 8 * 1024 * 1024);
  AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_FRAME_RATE, 30);
  AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 1);
  // Input-surface mode: the codec consumes frames from the surface; COLOR_FormatSurface
  // (0x7f000789) is required by hardware encoders such as OMX.amlogic.video.encoder.avc.
  AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, 0x7F000789);

  media_status_t status = AMediaCodec_configure(codec, format, nullptr, nullptr,
                                                AMEDIACODEC_CONFIGURE_FLAG_ENCODE);
  AMediaFormat_delete(format);
  if (status != AMEDIA_OK) {
    std::fprintf(stderr, "encoder: AMediaCodec_configure failed: %d\n", status);
    AMediaCodec_delete(codec);
    return nullptr;
  }

  ANativeWindow* window = nullptr;
  status = AMediaCodec_createInputSurface(codec, &window);
  if (status != AMEDIA_OK || !window) {
    std::fprintf(stderr, "encoder: AMediaCodec_createInputSurface failed: %d\n", status);
    AMediaCodec_delete(codec);
    return nullptr;
  }

  status = AMediaCodec_start(codec);
  if (status != AMEDIA_OK) {
    std::fprintf(stderr, "encoder: AMediaCodec_start failed: %d\n", status);
    AMediaCodec_delete(codec);
    return nullptr;
  }

  int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (fd < 0) {
    std::fprintf(stderr, "encoder: open(%s) failed: %s\n", path, std::strerror(errno));
    AMediaCodec_stop(codec);
    AMediaCodec_delete(codec);
    return nullptr;
  }

  // AMediaMuxer_new takes ownership of the file descriptor.
  AMediaMuxer* muxer = AMediaMuxer_new(fd, AMEDIAMUXER_OUTPUT_FORMAT_MPEG_4);
  if (!muxer) {
    std::fprintf(stderr, "encoder: AMediaMuxer_new failed\n");
    close(fd);
    AMediaCodec_stop(codec);
    AMediaCodec_delete(codec);
    return nullptr;
  }

  auto enc = std::unique_ptr<AndroidMediaEncoder>(new AndroidMediaEncoder());
  enc->codec_ = codec;
  enc->muxer_ = muxer;
  enc->window_ = window;
  return enc;
}

void AndroidMediaEncoder::Poll() {
  if (!codec_ || !muxer_) return;
  AMediaCodec* codec = static_cast<AMediaCodec*>(codec_);
  AMediaMuxer* muxer = static_cast<AMediaMuxer*>(muxer_);
  AMediaCodecBufferInfo info;
  while (DrainOutput(codec, muxer, track_index_, muxer_started_,
                     AMediaCodec_dequeueOutputBuffer(codec, &info, 0), info)) {
    // Keep draining until TRY_AGAIN_LATER / EOS.
  }
}

void AndroidMediaEncoder::Finish() {
  if (!codec_ || !muxer_) return;
  AMediaCodec* codec = static_cast<AMediaCodec*>(codec_);
  AMediaMuxer* muxer = static_cast<AMediaMuxer*>(muxer_);
  AMediaCodec_signalEndOfInputStream(codec);

  // Drain until EOS, bounded by a deadline so a stuck encoder cannot hang the demo.
  // (Some hardware encoders, e.g. this Amlogic AVC encoder, never emit the EOS output
  // buffer after signalEndOfInputStream; all produced frames are already muxed.)
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  AMediaCodecBufferInfo info;
  for (;;) {
    ssize_t status = AMediaCodec_dequeueOutputBuffer(codec, &info, 10000);
    if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
      if (std::chrono::steady_clock::now() > deadline) {
        std::fprintf(stderr, "encoder: EOS not signaled within 5 s; finalizing muxer\n");
        break;
      }
      continue;
    }
    if (status == AMEDIA_ERROR_END_OF_STREAM) break;
    DrainOutput(codec, muxer, track_index_, muxer_started_, status, info);
  }
}

AndroidMediaEncoder::~AndroidMediaEncoder() {
  if (codec_) {
    AMediaCodec_stop(static_cast<AMediaCodec*>(codec_));
    AMediaCodec_delete(static_cast<AMediaCodec*>(codec_));
    codec_ = nullptr;
  }
  if (muxer_) {
    if (muxer_started_) {
      AMediaMuxer_stop(static_cast<AMediaMuxer*>(muxer_));
    }
    AMediaMuxer_delete(static_cast<AMediaMuxer*>(muxer_));
    muxer_ = nullptr;
  }
}

#else  // !defined(__ANDROID__)

std::unique_ptr<AndroidMediaEncoder> AndroidMediaEncoder::Create(const char*, int, int) {
  // TODO(android-only): stub keeps host/CI builds green.
  return nullptr;
}
void AndroidMediaEncoder::Poll() {}
void AndroidMediaEncoder::Finish() {}
AndroidMediaEncoder::~AndroidMediaEncoder() {}

#endif  // defined(__ANDROID__)

}  // namespace native::ui
