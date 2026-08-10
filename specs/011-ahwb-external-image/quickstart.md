# Developer Quickstart: Android AHardwareBuffer ExternalImage

Min API 29 (Android 10). GPU backend = GLES/EGL. **Android is the only implemented platform**; host builds are guarded stubs (`// TODO(android-only)` + immediate return).

## Host CI: Contract + Regression Tests (macOS/Linux)

```bash
# Stub-contract and regression tests (host stays build-green, no rendering on host):
bazel test //tests:external_image_test //tests:surface_test //tests:render_test
```

## Android: Run the Real Closed Loop (compose → encode → MP4)

```bash
# Prerequisite: skia_gpu target + Android NDK toolchain + android platform (see plan step 0)
bazel build --config=android_arm64 //examples:external_image_demo
# Push and run on an API 29+ device/emulator:
adb push bazel-bin/examples/external_image_demo /data/local/tmp/
adb shell /data/local/tmp/external_image_demo
adb pull /data/local/tmp/external_image.mp4 /tmp/external_image.mp4
adb pull /data/local/tmp/external_image_cpu.png /tmp/external_image_cpu.png  # if exported
# Outputs: /tmp/external_image.mp4 (canvas hosted on AMediaCodec_createInputSurface, H.264 via AMediaMuxer)
#          /tmp/external_image_cpu.png (CPU snapshot, optional diagnostic export)
```

The loop: load `assets/photo/police.png` → decode RGBA → `AHwb::AllocateRgba` → `AHwb::WriteRgba` (injects row padding to exercise stride handling) → `HardwareBuffer::FromAHardwareBuffer` → `ExternalImage` → draw onto the **encoder-input-surface canvas** (`RenderContext::CreateFromMediaCodecInputSurface`) → `eglSwapBuffers` → `AMediaCodec` encodes → `AMediaMuxer` writes MP4 → decode back & pixel-diff vs source.

`--live` mode (30 Hz / 60 s) exercises the update path and bounded memory on device. `DumpPng` exports the source buffer for diagnostics.

## How It Works

```cpp
// 1. Load a PNG and decode to RGBA
auto img = Image::FromFile("assets/photo/police.png");   // or stbi_load for raw pixels

// 2. Create a real AHardwareBuffer and write pixels (Android)
auto* ahwb = AHwb::AllocateRgba(w, h);
AHwb::WriteRgba(ahwb, rgba.data(), w * 4);

// 3. Wrap it for the framework (non-owning)
auto hb = HardwareBuffer::FromAHardwareBuffer(ahwb);     // Android

// 4. Feed the widget
auto ext = std::make_unique<ExternalImage>(hb, Width{w}, Height{h});

// 5. GPU closed loop: canvas hosted on the encoder input surface (Android, GLES/EGL)
AMediaCodec* codec = AMediaCodec_createEncoderByType("video/avc");
//   AMediaCodec_configure(codec, format, nullptr, nullptr, AMEDIACODEC_CONFIGURE_FLAG_ENCODE);
ANativeWindow* window = nullptr;
AMediaCodec_createInputSurface(codec, &window);           // NDK, API 24+ → ANativeWindow*
auto ctx = RenderContext::CreateFromMediaCodecInputSurface(window, w, h);
ctx->MakeCurrent();
//   wrap ctx->surface as render target → root.Draw(canvas)
ctx->gr->flush();
ctx->SwapBuffers();                     // present → encoder dequeues (zero-copy) → AMediaMuxer → MP4

// Diagnostics (Android): export the displayed frame / source buffer to PNG
AHwb::DumpPng(ahwb, "/data/local/tmp/external_image_cpu.png");
```

## Key Files

- `src/framework/surface/ahwb.h/.cc` — Android AHardwareBuffer utility (lock/unlock/allocate/write/dump, stride-aware; host = stubs)
- `src/framework/surface/hardware_buffer.h/.cc` — cross-platform wrapper (geometry, kind, `operator==`, `FromMemory` data factory)
- `src/framework/surface/render_context.h/.cc` — RenderContext: GrDirectContext + EGL display/context/surface; `CreateFromMediaCodecInputSurface(ANativeWindow*)` (host = `nullptr`)
- `src/framework/render/image.cc` — `Image::FromBuffer` (Android CPU owned-copy path + GPU texture path; host = stub)
- `src/framework/surface/surface.cc` — `CreateFromBuffer` CPU/GPU backends; `RenderBackend` enum (host = stub)
- `src/framework/widgets/external_image.cc` — widget (rebuild guard via `operator==`)
- `examples/external_image_demo.cc` + `examples/android_media.cc` — closed loop + native `AMediaCodec`/`AMediaMuxer` encoder helper
- `tests/external_image_test.cc` — host contract/regression tests (stub contract, `operator==`, no-crash)
- `third_party/skia/BUILD.bazel` — `skia_gpu` target (prerequisite)

## Contracts

- `contracts/ahwb.md` — AHardwareBuffer utility contract
- `contracts/render-backend.md` — CPU/GPU backends, RenderContext, single-context rule
- `contracts/media-codec.md` — decode ingress (getOutputImage) + encode egress (createInputSurface), NDK native API
