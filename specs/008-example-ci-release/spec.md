# Feature Specification: Example, CI Polish & Release

**Feature Branch**: `008-example-ci-release`

**Created**: 2026-07-30

**Status**: Draft

**Input**: User description: "Phase 8: Example, CI Polish & Release"

## User Scenarios & Testing

### User Story 1 - Developer Runs the Hello World Example (Priority: P1)

A developer builds and runs a complete Hello World application that demonstrates the full MVP pipeline: create a widget tree with State data binding, run layout, render to a Skia surface, handle a button click, and automatically redraw when State changes.

**Why this priority**: The Hello World example is the primary integration point that validates all modules work together. It serves as both a demo and a smoke test for the entire framework.

**Independent Test**: The developer builds the example with `bazel build //examples:hello_world` and runs it, verifying it exits with code 0 and produces a valid output image with rendered widgets.

**Acceptance Scenarios**:

1. **Given** the Hello World example source, **When** built with `bazel build //examples:hello_world`, **Then** compilation succeeds without errors
2. **Given** the compiled Hello World binary, **When** executed, **Then** it exits with code 0
3. **Given** the Hello World output, **When** inspected, **Then** it shows correctly laid out widgets (Text, Button, Container) with proper colors and positioning
4. **Given** the Hello World example uses data binding, **When** a State property changes, **Then** the widget tree redraws with updated content

---

### User Story 2 - CI Pipeline Validates Full Build and Tests (Priority: P1)

The CI pipeline runs `bazel build //...` and `bazel test //...` on every push, across both macOS ARM64 and Linux x86_64 platforms. The visibility guard (`bazel query`) ensures Skia isolation rules are enforced.

**Why this priority**: CI is the quality gate for all contributions. Without cross-platform CI, regressions on Linux would go undetected until release time.

**Independent Test**: A developer pushes a change to the repository, and the CI pipeline automatically triggers, runs all builds and tests, reports green status, and enforces visibility constraints.

**Acceptance Scenarios**:

1. **Given** a CI pipeline trigger, **When** `bazel build //...` runs, **Then** all targets compile on both macOS and Linux
2. **Given** a CI pipeline trigger, **When** `bazel test //...` runs, **Then** all tests pass (20+)
3. **Given** CI enforcement, **When** a module outside `render/` or `surface/` depends on Skia, **Then** the CI visibility query fails
4. **Given** CI pipeline configuration, **When** a PR is opened, **Then** the PR gate runs the same checks

---

### User Story 3 - Developer Produces a Shared Library Release (Priority: P2)

A maintainer runs a build script that produces a shared library artifact (`libnative_ui_shared.dylib` / `.so`), creates a GitHub release with the artifact attached, and updates the changelog.

**Why this priority**: The shared library is the primary distribution artifact for consumers. Without a build script and release process, users cannot integrate the framework into their projects.

**Independent Test**: A maintainer runs the release script locally, verifies the shared library is produced in `dist/`, and confirms the GitHub release workflow creates a tagged release with the artifact.

**Acceptance Scenarios**:

1. **Given** the build script, **When** executed, **Then** `libnative_ui_shared.dylib` (macOS) or `.so` (Linux) is produced in `dist/`
2. **Given** a new version tag, **When** the release workflow runs, **Then** a GitHub Release is created with the shared library attached
3. **Given** a release, **When** the CHANGELOG is reviewed, **Then** it documents MVP changes since the last release

---

### Edge Cases

- What happens when the CI matrix encounters a platform-specific Skia linkop issue?
- What happens when the example has no event system attached (click does nothing)?
- What happens when the shared library is linked into a project with incompatible C++ settings?
- What happens when `bazel build //...` encounters a dependency resolution failure?
- What happens when the example output image is empty (no widgets rendered)?

## Requirements

### Functional Requirements

- **FR-001**: Hello World example MUST build a widget tree with at least Container, Text, and Button widgets connected to a State with data binding
- **FR-002**: Hello World example MUST run layout (Measure/Arrange), render to a Skia surface via Canvas, and export the result as a PNG image
- **FR-003**: Hello World example MUST handle a button click that updates a State property and triggers an automatic redraw
- **FR-004**: A full pipeline integration test MUST cover Container → FlexLayout → Canvas draw → Skia surface → PNG encode
- **FR-005**: CI pipeline MUST run `bazel build //...` on both macOS ARM64 and Linux x86_64
- **FR-006**: CI pipeline MUST run `bazel test //...` across all test targets
- **FR-007**: CI pipeline MUST include a `bazel query` visibility guard ensuring only `render/` and `surface/` modules depend on Skia
- **FR-008**: A build script (`scripts/build_shared.sh`) MUST produce the shared library artifact in `dist/`
- **FR-009**: A GitHub Actions release workflow MUST create a GitHub Release on version tags with the shared library attached
- **FR-010**: CHANGELOG.md MUST be populated with MVP changes
- **FR-011**: CI caching MUST be enabled to speed up subsequent builds
- **FR-012**: A CI status badge MUST be present in the repository README

### Key Entities

- **Hello World Example**: A self-contained C++ source file demonstrating the full MVP pipeline — widget tree construction, data binding, layout, rendering, and event handling.
- **Shared Library Build Script**: A shell script that builds the public umbrella target and copies the output to a `dist/` directory.
- **CI Pipeline**: GitHub Actions workflow files (ci.yml, pr.yml, release.yml) that automate build, test, and release across platforms.
- **Release Workflow**: A GitHub Actions workflow triggered by version tags that builds the shared library and creates a GitHub Release.
- **CHANGELOG**: A markdown file documenting release history, starting with the MVP v0.1.0 entry.
- **Visibility Query**: A Bazel query expression that enforces Skia isolation rules.

## Success Criteria

### Measurable Outcomes

- **SC-001**: Hello World example compiles and runs in under 30 seconds on a developer machine
- **SC-002**: The example output image correctly renders widgets with verified layout positions
- **SC-003**: A button click in the example triggers a State update and the output image differences are measurable
- **SC-004**: `bazel build //...` completes on macOS ARM64 in under 5 minutes
- **SC-005**: `bazel test //...` passes with 100% pass rate (20+ tests)
- **SC-006**: The visibility query (`bazel query`) correctly passes on valid code and fails on violations
- **SC-007**: The shared library build script produces a usable `.dylib`/`.so` artifact under 5MB
- **SC-008**: CI pipeline reports green status with caching within 10 minutes

## Assumptions

- The Hello World example outputs a PNG file — no interactive window system required
- CI uses GitHub Actions with self-hosted or GitHub-hosted runners for both macOS and Linux
- Bazel remote cache (or local disk cache) is used for CI caching
- The shared library targets the same C++ ABI as the framework's C++17 configuration
- GitHub Releases are created manually via the workflow dispatch or tag push
- The visibility query is integrated into the CI workflow as a separate step
- CHANGELOG follows Keep a Changelog format
- Linux CI testing covers Ubuntu x86_64 (the primary Linux target)
