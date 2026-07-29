# Feature Specification: Project Scaffolding & Skia Spike

**Feature Branch**: `001-project-scaffold`

**Created**: 2026-07-28

**Status**: Draft

**Input**: User description: "P1 - Project Scaffolding & Skia Spike"

## User Scenarios & Testing

### User Story 1 - Developer Builds Project from Source (Priority: P1)

A developer clones the repository and builds the entire project with a single command, confirming the build system is fully operational.

**Why this priority**: The ability to build the project is the foundational gate for all subsequent development. Without a working build, no other work can proceed.

**Independent Test**: A fresh clone of the repository can be built entirely from source, producing all expected build artifacts without manual intervention beyond the single build command.

**Acceptance Scenarios**:

1. **Given** a developer has cloned the repository on a supported platform (macOS ARM64 or Linux x86_64), **When** they run the full build command, **Then** all targets compile successfully with zero errors
2. **Given** the build completes, **When** the developer inspects the output, **Then** the build artifacts for all expected modules (core, layout, render, surface, widgets, event, public) are present
3. **Given** a developer has made changes to a single module, **When** they rebuild, **Then** only the affected module and its dependents are recompiled (incremental build works)

---

### User Story 2 - Integrator Depends on Native UI Library (Priority: P1)

An integrator adds native_ui as a dependency of their own Bazel project and successfully links against it.

**Why this priority**: The project's primary distribution model is as a Bazel library dependency. This story validates that the public API target is correctly exposed and linkable.

**Independent Test**: An external Bazel project can declare a dependency on native_ui and successfully compile and link a binary that calls a native_ui function.

**Acceptance Scenarios**:

1. **Given** an external Bazel project, **When** it declares a dependency on native_ui's public target and builds, **Then** the dependency resolves and links successfully
2. **Given** the external project, **When** it includes the native_ui umbrella header and compiles, **Then** no header resolution errors occur

---

### User Story 3 - Skia Spike Validates Rendering Pipeline (Priority: P1)

A developer runs the Skia spike binary and confirms that Skia can create a surface, draw a shape, and produce a valid output file.

**Why this priority**: Skia integration is the highest technical risk in the project. Validating it early prevents wasted effort if the dependency cannot be integrated on the target platform.

**Independent Test**: A minimal standalone binary that exercises Skia's core drawing pipeline (surface creation, canvas drawing, pixel output encoding) compiles, links, and runs successfully, producing a valid image file.

**Acceptance Scenarios**:

1. **Given** the build system is set up, **When** the developer builds the spike binary, **Then** it compiles and links without errors
2. **Given** the spike binary runs, **When** it executes, **Then** it produces a valid PNG image file as output
3. **Given** the output image, **When** inspected, **Then** it contains the expected drawn content (e.g., a colored rectangle)

---

### User Story 4 - Developer Runs All Tests (Priority: P2)

A developer runs all project tests to confirm the test infrastructure works, even though no application-level tests exist yet.

**Why this priority**: Test infrastructure must be operational before any feature tests are written. This ensures the test pipeline works end-to-end.

**Independent Test**: The test command executes all registered tests and reports zero failures.

**Acceptance Scenarios**:

1. **Given** the build is complete, **When** the developer runs the test command, **Then** test discovery works and all tests execute
2. **Given** no tests are registered yet, **When** the test command runs, **Then** it reports success (empty test suite passes)

---

### User Story 5 - Yoga+Skia Spike Validates Flex Layout with Margin (Priority: P1)

A developer runs the Yoga+Skia spike binary and confirms that Yoga computes flexbox layout (with margin) and Skia renders the result correctly.

**Why this priority**: Validates the combined Yoga+Skia pipeline — Yoga layout calculation + Skia rendering — which is the core rendering path for all downstream widgets. Margin is a key flexbox feature that directly impacts visual spacing.

**Independent Test**: A binary that creates a Yoga node tree with margin settings, calculates layout, and renders each node as a colored rectangle via Skia, produces a valid PNG image showing proper spacing.

**Acceptance Scenarios**:

1. **Given** the build system is set up, **When** the developer builds the yoga_spike binary, **Then** it compiles and links without errors
2. **Given** the spike binary runs, **When** it executes, **Then** it produces a valid PNG image file as output
3. **Given** the output image, **When** inspected, **Then** it contains at least two flex layouts (row + column) with colored rectangles spaced apart by visible margins
4. **Given** the output image, **When** measured, **Then** each child rectangle's position matches the position computed by Yoga including margin offsets

---

### Edge Cases

- What happens when the build is attempted on an unsupported platform (e.g., Windows x86_64)?
- How does the build system handle missing system dependencies (e.g., Xcode command line tools on macOS)?
- What happens when network access is unavailable and dependencies need to be fetched from cache?
- How does the system handle a Skia API change that breaks the spike build?
- What happens when Yoga layout produces zero-sized nodes due to conflicting constraints?
- How does the system handle overlapping margins between adjacent flex items?

## Requirements

### Functional Requirements

- **FR-001**: A developer must be able to build the entire project from source with a single command, producing all module targets
- **FR-002**: The project must expose a public API target that external Bazel projects can declare as a dependency
- **FR-003**: A minimal Skia integration binary must compile, link, and execute, producing a valid image output
- **FR-004**: The build must support at least macOS ARM64 and Linux x86_64 as target platforms
- **FR-005**: Module source directories must exist as stubs for: core types, layout engine, render wrapper, surface, widgets, event, and public API
- **FR-006**: Each module stub must be independently compilable as an empty library target
- **FR-007**: The project root must provide a convenient alias that resolves to the public API target
- **FR-008**: Third-party dependencies (Skia, Yoga, googletest, skylib, stblib) must be fetchable and resolvable via the build system
- **FR-009**: Platform-specific build settings (compiler flags, link options) must be correctly applied per target platform
- **FR-010**: The Skia spike binary must use Skia's canvas API to draw content and encode the result as a PNG file
- **FR-011**: The Yoga+Skia spike binary must combine Yoga flexbox layout (with margin) and Skia rendering, producing a PNG that visually demonstrates margin spacing in both row and column directions

### Key Entities

- **Build Target**: A named buildable unit within the project that produces a library or binary artifact
- **Platform**: A target operating environment defined by OS and CPU architecture combination (macOS ARM64, Linux x86_64)
- **Third-Party Dependency**: An external library fetched and managed by the build system, not part of the project source
- **Module**: A logical grouping of source code within the project (core, layout, render, surface, widgets, event, public)
- **Public API Target**: The designated build target that external consumers depend on, exposing the full project interface
- **Spike Binary**: A minimal executable that validates a high-risk integration (Skia, Yoga+Skia) compiles, links, and functions correctly
- **Flex Layout**: A layout mode based on the W3C Flexbox specification, computed by Yoga, supporting direction, alignment, and margin/padding spacing

## Success Criteria

### Measurable Outcomes

- **SC-001**: A developer can build all project targets from a fresh clone using a single command in under 5 minutes (first build including dependency download)
- **SC-002**: The Skia spike binary compiles, links, and executes successfully on both macOS ARM64 and Linux x86_64 platforms
- **SC-003**: An external Bazel project can declare a dependency on native_ui and link a binary without errors
- **SC-004**: All module stub targets compile successfully, confirming the module directory structure is correct
- **SC-005**: Incremental builds after a single source change complete in under 30 seconds
- **SC-006**: The build system correctly applies platform-specific settings (verified by inspecting compiler/linker flags per platform)
- **SC-007**: The Yoga+Skia spike binary compiles, links, and executes successfully, producing a PNG with two distinct flex layout sections showing correct margin spacing
- **SC-008**: The output PNG from the Yoga+Skia spike visually confirms that margin creates predictable gaps between flex children in both row and column directions

## Assumptions

- Developers have Bazel 6.5.0 installed or the project pins it via `.bazelversion`
- The primary development platforms are macOS ARM64 and Linux x86_64; other platforms are out of scope for P1
- The project will use the same Bazel conventions as graph_runtime (a sibling project)
- Skia will be integrated as an external dependency via archive download, not as a submodule
- Network access is available for the first build to fetch third-party dependencies
- External consumers of native_ui also use Bazel as their build system
- The public API will use a C++ umbrella header pattern for simple inclusion
