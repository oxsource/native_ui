# Research: Example, CI Polish & Release

**Date**: 2026-07-30

## Decisions

### Hello World: Self-Contained .cc File with Public API Only

- **Decision**: The Hello World example is a single `hello_world.cc` file that includes only public API headers (`native_ui/core.h`, `native_ui/widgets.h`, `native_ui/render.h`, `native_ui/surface.h`). No internal headers.
- **Rationale**: The example serves as both a demo and an integration test of the public API. Using internal headers would bypass the public contract and risk breaking consumer code.
- **Alternatives considered**: Multi-file example (rejected: simpler is better for a Hello World); Example with framework internals (rejected: defeats purpose of public API validation)

### Example Output: PNG File (Not Interactive Window)

- **Decision**: Hello World renders to an off-screen Skia surface and encodes the result as a PNG file. No window system, event loop, or interactive display.
- **Rationale**: PNG output is CI-friendly, does not require a display server, and can be visually inspected in CI artifacts or compared pixel-by-pixel in tests.
- **Alternatives considered**: Interactive window via GLFW/SDL (rejected: adds platform dependency, not CI-friendly); Simple stdout text output (rejected: doesn't validate rendering)

### CI Matrix: macOS ARM64 + Linux x86_64

- **Decision**: CI runs the full build and test suite on both macOS ARM64 and Linux x86_64. PR checks run on both platforms. The release build targets macOS (dylib) and Linux (so) separately.
- **Rationale**: The framework targets both platforms. Linux CI is essential since many developers work on Linux. macOS ARM64 is the primary development platform.
- **Alternatives considered**: Single platform only (rejected: would miss cross-platform bugs); Add Windows (deferred: out of MVP scope)

### Visibility Guard: Bazel Query Integrated Into CI

- **Decision**: CI runs `bazel query 'somepath(//src/framework/..., @skia//:skia)'` and checks that only paths through `render/` or `surface/` are returned.
- **Rationale**: This enforces the Skia isolation architecture — no module outside render/surface should depend on Skia. The query is a hard CI gate.
- **Alternatives considered**: Manual code review (rejected: not automated); Compiler-based isolation (not practical)

### Shared Library: cc_shared_library in Public Module

- **Decision**: The shared library target is added to `src/framework/public/BUILD.bazel` using Bazel's `cc_shared_library` rule (or a `cc_binary` with `linkshared = True`). The build script copies the output to `dist/`.
- **Rationale**: Bazel's shared library rules handle platform-specific extensions (.dylib vs .so). The build script is a convenience wrapper for non-Bazel consumers.
- **Alternatives considered**: Only static library (rejected: shared library is the standard distribution format); Manual dylib creation (rejected: error-prone)

### CHANGELOG: Keep a Changelog Format

- **Decision**: CHANGELOG.md follows the Keep a Changelog format with version v0.1.0 for MVP. Each section lists additions, changes, and fixes.
- **Rationale**: Keep a Changelog is the industry standard for manually-maintained changelogs. It's readable by both humans and tools.
- **Alternatives considered**: Automated changelog from commits (rejected: noisy, less readable); No changelog (rejected: required for release)
