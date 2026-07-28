# Data Model: Project Scaffolding & Skia Spike

**Date**: 2026-07-28

## Entities

### Bazel Workspace

The top-level container for the project. Identified by `workspace(name = "native_ui")`. All paths are relative to the workspace root.

- **name**: `native_ui`
- **root**: `<repository_root>/native_ui/`
- **conventions**: Follows graph_runtime layout patterns

### Module

A logical grouping of source code within the framework. Each module is a Bazel package under `src/framework/`.

| Module | Path | Visibility | Dependencies |
|--------|------|------------|--------------|
| core | `src/framework/core` | `__subpackages__`, `tests` | None |
| layout | `src/framework/layout` | `__subpackages__`, `tests` | core, @caflex |
| render | `src/framework/render` | `__subpackages__`, `tests` | core, @skia |
| surface | `src/framework/surface` | `__subpackages__`, `tests` | core, @skia, platform headers |
| widgets | `src/framework/widgets` | `__subpackages__`, `tests` | core, layout, render, event |
| event | `src/framework/event` | `__subpackages__`, `tests` | core |
| public | `src/framework/public` | `//visibility:public` | All modules |

### Platform

A target environment defined by OS + CPU architecture.

- **macOS ARM64**: `platforms:macos_arm64` — constraint_values: `@platforms//os:macos`, `@platforms//cpu:aarch64`
- **Linux x86_64**: `platforms:linux_x86_64` — constraint_values: `@platforms//os:linux`, `@platforms//cpu:x86_64`

### Build Target

A named buildable unit (cc_library, cc_binary, cc_test) with dependencies, visibility, and platform-specific settings.

- **type**: One of: library (cc_library), binary (cc_binary), test (cc_test)
- **visibility**: Controls which packages may depend on this target
- **target_compatible_with**: Optional platform constraint

### Third-Party Dependency

An external library fetched and managed by the build system, not part of project source.

| Name | Type | Source | Purpose |
|------|------|--------|---------|
| skia | http_archive | GitHub | 2D graphics rendering library |
| caflex | http_archive | GitHub | Flexbox layout engine (header-only) |
| googletest | http_archive | GitHub | Unit testing framework |
| bazel_skylib | http_archive | GitHub | Bazel helper library |

### Spike Binary

A minimal executable that validates a high-risk integration compiles, links, and functions correctly.

- **name**: `skia_spike`
- **location**: `src/spike/`
- **dependencies**: `@skia//:skia`
- **success criteria**: Compiles, links, runs, produces valid PNG output

## State Transitions

N/A — P1 is a build system scaffolding phase. No runtime state transitions exist.

## Validation Rules

- **Visibility Rule**: No target outside `src/framework/render` or `src/framework/surface` may depend on `@skia` directly
- **Platform Rule**: Platform-specific linkopts must only activate on their respective platform (macOS frameworks on Apple platforms, empty on Linux)
- **Dep Rule**: All external dependencies must be declared in `native_ui_deps.bzl` and use `native.existing_rule()` guards
- **Stub Rule**: All 7 module BUILD.bazel files must be present and compilable (even if empty)
