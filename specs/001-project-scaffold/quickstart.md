# Quickstart: native_ui Project Scaffolding

**Date**: 2026-07-28

## Prerequisites

- Bazel 6.5.0 (or Bazelisk which auto-selects version from `.bazelversion`)
- Xcode Command Line Tools (macOS) or build-essential (Linux)
- Git

## Clone & Build

```sh
git clone <repository_url>
cd native_ui

# Build everything (first build downloads dependencies)
bazel build //...

# Run the Skia spike to validate rendering integration
bazel run //src/spike:skia_spike

# Run all tests (initially empty test suite)
bazel test //...
```

## Verify

- `bazel build //...` succeeds with zero errors
- `bazel run //src/spike:skia_spike` produces a valid PNG file
- The root alias `//:native_ui` resolves to the public API target

## Project Layout

```
native_ui/              ← Bazel workspace root
├── BUILD.bazel         ← Root alias
├── WORKSPACE           ← Workspace definition
├── *.bzl               ← Build files
├── platforms/          ← Platform definitions
├── third_party/        ← Third-party BUILD wrappers
├── src/framework/      ← Framework source modules
│   ├── core/           ← Core types (stub)
│   ├── layout/         ← Layout engine (stub)
│   ├── render/         ← Skia wrapper (stub)
│   ├── surface/        ← Platform surface (stub)
│   ├── widgets/        ← Widget controls (stub)
│   ├── event/          ← Event handling (stub)
│   └── public/         ← Public API (has native_ui_export.h)
├── src/spike/          ← Skia spike binary
└── tests/              ← Test targets
```

## External Dependency

To use native_ui from another Bazel project, add to `WORKSPACE`:

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "native_ui",
    urls = ["https://github.com/yourorg/native_ui/archive/v0.1.0.tar.gz"],
    sha256 = "<sha256>",
)

# Then in BUILD:
# deps = ["@native_ui//:native_ui"]
```

## CI Commands

```sh
bazel build //...                  # Full build
bazel test //...                   # All tests
bazel query 'somepath(//src/framework/..., @skia//:skia)'  # Verify Skia isolation
```
