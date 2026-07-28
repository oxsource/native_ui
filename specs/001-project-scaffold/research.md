# Research: Project Scaffolding & Skia Spike

**Date**: 2026-07-28

## Overview

All technical decisions for P1 are documented in `project_bootstrap.md` and `roadmap.md`. No NEEDS CLARIFICATION items existed in the spec. This document consolidates the decisions, rationale, and alternatives for the scaffolding phase.

---

## Decision Log

### Build System: Bazel 6.5.0

- **Decision**: Use Bazel 6.5.0 as the build system
- **Rationale**: Required for reproducible multi-platform builds; same toolchain as graph_runtime (sibling project), reducing team context switching
- **Alternatives considered**: CMake (rejected: weaker dependency management, no sandboxing); Make (rejected: non-reproducible, no multi-platform support); Buck2 (rejected: smaller ecosystem, less documentation)

### Platform Targets: macOS ARM64 + Linux x86_64

- **Decision**: Support macOS ARM64 (development) and Linux x86_64 (CI/release)
- **Rationale**: Team primarily develops on macOS; CI runs on Linux; both are required for cross-platform validation
- **Alternatives considered**: macOS-only (rejected: project explicitly cross-platform); Windows (deferred: not in scope for v1)

### Language Standard: C++17

- **Decision**: Use C++17 with `-fvisibility=hidden`
- **Rationale**: C++17 provides fold expressions, structured bindings, `if constexpr` — all needed for the tagged-parameter pattern. Matches graph_runtime.
- **Alternatives considered**: C++20 (rejected: weaker toolchain support on all target platforms); C++14 (rejected: no fold expressions, making tagged-parameter pattern verbose)

### Skia Integration: http_archive + BUILD wrapper

- **Decision**: Fetch Skia via `http_archive` in WORKSPACE, provide a `cc_library` wrapper in `third_party/skia/BUILD.bazel`
- **Rationale**: Most maintainable approach; does not require submodule maintenance; BUILD wrapper can pin exact headers/sources needed
- **Alternatives considered**: Git submodule (rejected: version management overhead); system-installed Skia (rejected: non-reproducible across platforms); Bazel `new_local_repository` (rejected: requires manual download/cache)

### Skia Spike: cc_binary with Surface + Canvas + PNG encode

- **Decision**: Write a minimal `skia_spike.cc` that creates an `SkSurface`, draws a rectangle via `SkCanvas`, and encodes to PNG via `SkPngEncoder`
- **Rationale**: Validates the full compile-link-execute pipeline for the most complex dependency; exercises Skia's include paths, linkopts, and runtime library loading
- **Alternatives considered**: Compile-only test (rejected: doesn't validate linking or runtime); no spike (rejected: highest risk item must be validated before P2)

### Layout Engine: Yoga (replacing caflex)

- **Decision**: Use Facebook Yoga v3.2.1 as the flexbox layout engine
- **Rationale**: Yoga is the industry-standard flexbox implementation, maintained by Meta, used in React Native. caflex was initially chosen but does not exist as a maintained library.
- **Alternatives considered**: caflex (rejected: unmaintained/not found); custom implementation (rejected: unnecessary effort when Yoga is battle-tested)

### Dependency Management: Single deps.bzl

- **Decision**: Centralize all external dependency declarations in `native_ui_deps.bzl` with a single `native_ui_setup()` call
- **Rationale**: Single source of truth for all third-party deps; easy to add/update/audit
- **Alternatives considered**: Inline in WORKSPACE (rejected: cluttered, hard to maintain); one .bzl per dep (rejected: over-engineering for current scale)

### Testing Framework: googletest

- **Decision**: Use googletest 1.14.0 via http_archive
- **Rationale**: Industry standard C++ testing framework; well-integrated with Bazel; used by graph_runtime
- **Alternatives considered**: Catch2 (rejected: less Bazel ecosystem integration); doctest (rejected: less mature, smaller community)

### Module Visibility: __subpackages__ isolation

- **Decision**: Each module's `cc_library` uses `visibility = ["//src/framework:__subpackages__", "//tests:__subpackages__"]`
- **Rationale**: Prevents external code from depending on internal modules; enforces module boundaries; only `public/` target has `//visibility:public`
- **Alternatives considered**: All public visibility (rejected: no encapsulation); fine-grained per-target visibility (rejected: excessive maintenance)

---

## Risk Assessment

| Risk | Mitigation | Status |
|------|------------|--------|
| Skia build integration failure | Spike in P1 validates before P2 | Active — spike must pass |
| Platform-specific linkopts incorrect | Spike tests both platforms | Active — CI matrix needed |
| Bazel version incompatibility | .bazelversion pins exact version | Mitigated |
| Large initial dependency download | Document as expected in SC-001 (5 min budget) | Accepted |
