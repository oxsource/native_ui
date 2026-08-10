#pragma once

#include <memory>

struct ANativeWindow;

namespace native::ui {

// H.264 MediaCodec encoder (AMediaCodec) + AMediaMuxer MP4 writer, driven through
// the native NDK C API (<media/NdkMediaCodec.h>, <media/NdkMediaMuxer.h>) — no JNI.
// Android-only (AMediaCodec_createInputSurface is API 24+; min API 29). The framework
// renders onto input_window() (an ANativeWindow), and Poll()/Finish() drain the encoded
// output into the muxer. On host builds Create() returns nullptr (guarded stub).
class AndroidMediaEncoder {
public:
  // Creates an H.264 encoder producing an MP4 at `path` for a `width`x`height` input
  // surface. Returns nullptr on failure (or on non-Android hosts).
  static std::unique_ptr<AndroidMediaEncoder> Create(const char* path, int width, int height);

  // The surface the framework's RenderContext should be created on (createInputSurface).
  struct ANativeWindow* input_window() const { return window_; }

  // Drains any available encoded buffers into the muxer.
  void Poll();

  // Signals end-of-input, drains remaining output, finalizes the MP4.
  void Finish();

  ~AndroidMediaEncoder();

private:
  AndroidMediaEncoder() = default;

  // Unused on host (stub implementation) — [[maybe_unused]] keeps the build clean.
  [[maybe_unused]] void* codec_ = nullptr;   // AMediaCodec*
  [[maybe_unused]] void* muxer_ = nullptr;   // AMediaMuxer*
  [[maybe_unused]] struct ANativeWindow* window_ = nullptr;
  [[maybe_unused]] int track_index_ = -1;
  [[maybe_unused]] bool muxer_started_ = false;
};

}  // namespace native::ui
