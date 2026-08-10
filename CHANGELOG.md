# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Initial project scaffolding and build system (Bazel 6.5.0)
- Skia integration spike (surface creation, draw rect, PNG encode)
- Yoga integration spike (flexbox layout with margin, Skia render)
- Architecture & engineering design (8-module architecture, ViewModel data binding, threading, logging slot, frame clock)
- API interface contracts (widget, layout, render, event, state)
- CI/CD pipeline (build, test, format, lint, Skia isolation query)
- Release process and spec-kit templates
- **Android AHardwareBuffer ExternalImage** (Android-only, min API 29, GLES/EGL):
  - `HardwareBuffer` wrapper (Kind, geometry, `operator==`, `FromMemory` data factory)
  - `AHwb` AHardwareBuffer utility layer (lock/unlock/describe/allocate/write/release,
    CPU owned-copy + GPU zero-copy image, PNG diagnostic)
  - `RenderBackend` (`kCPU`/`kGPU`) + `Surface::CreateFromBuffer`/`Image::FromBuffer`
  - `RenderContext` (GrDirectContext + EGL) hosted on `AMediaCodec_createInputSurface`
  - `ExternalImage` live-update rebuild guard (`operator==`, FR-003/FR-007)
  - `examples/external_image_demo` closed loop: PNG → AHardwareBuffer → widget →
    encoder-surface canvas → `AMediaCodec`/`AMediaMuxer` → MP4 (native NDK, no JNI),
    plus `--live` mode (30 Hz / 60 s) and diagnostic PNG export
  - Bazel: Android platform + `rules_android_ndk` toolchain (`--config=android_arm64`),
    `skia_gpu` target, host guarded-stub contract tests
