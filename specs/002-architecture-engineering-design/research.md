# Research: Architecture & Engineering Design

**Date**: 2026-07-29

## Overview

This research consolidates architectural design decisions for the native_ui framework based on the project bootstrap document, roadmap, established build system (P1), and industry best practices for C++ UI frameworks.

---

## Decision Log

### Module Architecture: 7-Module Layered Design

- **Decision**: Organize the framework into 7 modules — core, layout, render, surface, widgets, event, public — with strictly enforced dependency direction
- **Rationale**: Each module has a single responsibility and clear dependency boundary. `core` has zero dependencies. `layout` depends only on core + Yoga. `render` depends only on Skia. `widgets` is the integration hub. `public` is the aggregation facade. This prevents circular dependencies and enforces Skia isolation.
- **Alternatives considered**: Monolithic single module (rejected: no separation of concerns); Flat module structure without layering (rejected: would allow Skia deps to leak)
- **Reference**: `native_ui/doc/project_bootstrap.md §3.2`

### Widget Inheritance: Abstract Base + Tagged Parameters

- **Decision**: Define `Widget` as an abstract base class with `Draw(Canvas&)` pure virtual. Use tagged-parameter constructor pattern for all concrete widgets.
- **Rationale**: The tagged-parameter pattern enables declarative widget tree construction without setter chains. C++17 fold expressions make the pattern zero-cost. The abstract base allows polymorphic widget lists.
- **Alternatives considered**: CRTP-based static polymorphism (rejected: incompatible with heterogeneous widget lists); Builder pattern (rejected: verbose, not declarative)
- **Reference**: `native_ui/doc/project_bootstrap.md §3.4, §3.6`

### Layout Engine: Yoga Wrapper

- **Decision**: Wrap Yoga's C API in a `FlexLayout` C++ class that accepts tagged parameters and exposes `Measure()` / `Arrange()` methods
- **Rationale**: Yoga is the battle-tested industry-standard flexbox implementation. The wrapper provides a native_ui-idiomatic API while hiding Yoga's C interface.
- **Alternatives considered**: Direct Yoga C API exposure (rejected: not idiomatic for C++ consumers); Custom flexbox implementation (rejected: unnecessary effort, maintenance burden)
- **Reference**: `native_ui/doc/project_bootstrap.md §3.5`

### Error Handling: StatusOr + Logging, No Exceptions

- **Decision**: Use a `StatusOr<T>` pattern for recoverable errors (layout failures, resource loading). Use logging for non-fatal diagnostics. No C++ exceptions.
- **Rationale**: Consistent with Google C++ Style Guide (exceptions prohibited). StatusOr provides explicit error propagation without hidden control flow. Logging allows diagnostics without disrupting rendering.
- **Alternatives considered**: C++ exceptions (rejected: Google style violation, unpredictable in render loops); Error codes via out-params (rejected: verbose, easy to ignore)
- **Reference**: Google C++ Style Guide — Exceptions

### Memory Model: unique_ptr Ownership + Raw Pointer Observation

- **Decision**: `Widget` and child lists use `std::unique_ptr` for exclusive ownership. Non-owning references use raw pointers (`Widget*`). No `std::shared_ptr` unless truly shared ownership is required.
- **Rationale**: Clear ownership semantics. The widget tree has a single root, and each widget owns its children. `FindById` and event dispatch need observation only, not ownership.
- **Alternatives considered**: `std::shared_ptr` everywhere (rejected: unclear ownership, performance overhead); Raw pointers for ownership (rejected: leak-prone)
- **Reference**: Google C++ Style Guide — Ownership

### Rendering: RAII Canvas Wrapper

- **Decision**: Create a `Canvas` RAII wrapper over `SkCanvas` that performs automatic save/restore on scope exit.
- **Rationale**: `SkCanvas` requires explicit save/restore calls — forgetting either causes visual bugs or crashes. RAII eliminates this class of error entirely.
- **Alternatives considered**: Direct `SkCanvas` usage (rejected: error-prone save/restore management)
- **Reference**: `native_ui/doc/project_bootstrap.md §3.7`

### Event System: HitTest + Bubble/Capture

- **Decision**: Implement W3C-style event dispatch with bubble (leaf→root) and capture (root→leaf) phases. `HitTester` performs DFS hit-testing on the widget tree.
- **Rationale**: Bubble/capture matches the standard DOM event model familiar to web developers. DFS hit-testing handles overlapping widgets correctly.
- **Alternatives considered**: Flat hit-test area map (rejected: cannot handle arbitrary widget nesting); Leaf-only dispatch (rejected: container widgets need event handling too)

### CI/CD: GitHub Actions + Bazel

- **Decision**: Use GitHub Actions for CI/CD with a matrix build (macOS ARM64 + Linux x86_64). Pipeline: build → test → format → lint → visibility query.
- **Rationale**: GitHub Actions is the standard CI for GitHub-hosted projects. Matrix build ensures both target platforms stay green. Bazel's remote caching can accelerate subsequent runs.
- **Alternatives considered**: Jenkins (rejected: too heavy for project scale); CircleCI (rejected: less GitHub integration); Self-hosted (rejected: maintenance overhead)
- **Reference**: `native_ui/doc/roadmap.md §Phase 2`

### Release Process: Semantic Versioning + Shared Library

- **Decision**: Use SemVer (MAJOR.MINOR.PATCH) versioning. Produce a shared library artifact (`.dylib` / `.so`) via `cc_binary(linkshared=True)`.
- **Rationale**: SemVer is the industry standard for library versioning. Shared library allows non-Bazel consumers to depend on native_ui.
- **Alternatives considered**: Git tag-only releases (rejected: no artifact for non-Bazel consumers); Static library only (rejected: bloat for downstream consumers)
- **Reference**: `native_ui/doc/project_bootstrap.md §4.10`

### Testing Strategy: Unit + Integration + Golden

- **Decision**: Use googletest for unit and integration tests. Use golden image comparison (PNG hash) for render tests.
- **Rationale**: googletest is the Bazel-compatible C++ testing standard. Golden tests catch visual regressions without manual pixel inspection.
- **Alternatives considered**: Catch2 (rejected: less Bazel integration); Screenshot diffing (rejected: too heavy, platform-dependent)
- **Reference**: `native_ui/doc/roadmap.md §Phase 5, §Phase 6`

### Threading Model: Main Thread + Worker Threads (React-Inspired Batch)

- **Decision**: Rendering (Skia draw), layout calculation, and event dispatch execute exclusively on the main thread. Business logic and data processing run on worker threads. ViewModel acts as the cross-thread bridge — worker threads update ViewModel properties, main thread observes changes and triggers redraw. **ViewModel property changes are automatically batched within a single frame** (React-style `setState` batch), so multiple mutations trigger only one layout + render pass.
- **Rationale**: Skia's raster API is not thread-safe for concurrent draw calls. Keeping layout and rendering on one thread avoids synchronization overhead on the hot path. Worker threads keep the UI responsive during data processing. ViewModel's property notification provides a natural synchronization boundary. The batch model eliminates the need for explicit `nextTick` — the frame loop naturally coalesces all pending changes.
- **Alternatives considered**: Multi-threaded rendering (rejected: Skia thread safety complexity, marginal benefit for 2D UI); All-on-main-thread (rejected: blocking on I/O/computation freezes UI); Render on separate thread (rejected: adds cross-thread Skia surface management complexity)

### Logging: Slot Interface Pattern

- **Decision**: Framework defines an abstract `LogSink` base class with `Log(level, message, metadata)` as the only required method. No default implementation. If no LogSink is plugged in, log calls are no-ops (zero overhead when not used). The consumer creates a subclass and registers it at startup.
- **Rationale**: Slot interface decouples the framework from any specific logging library. The user explicitly stated they will design a separate reusable logging module — this pattern allows that module to plug into native_ui without changes to the framework.
- **Alternatives considered**: Built-in spdlog (rejected: adds dependency, user wants their own logger); Built-in fprintf/stderr (rejected: insufficient for production); No logging (rejected: debugging impossible)

### Data Binding: React-Inspired ViewModel Pattern

- **Decision**: Adopt React-style unidirectional data flow (props ↓, events ↑) with a `ViewModel` base class providing property change notification. Widgets bind to ViewModel properties and auto-trigger `RequestRedraw` on change. No Vue-style computed properties or deep reactivity system.
- **Rationale**: React's model aligns naturally with the existing tagged-parameter props pattern and event bubbling. Simpler to implement than Vue's proxy-based reactivity. Avoids template/DSL complexity — ViewModels are plain C++ objects.
- **Alternatives considered**: Vue-style proxy reactivity (rejected: C++ lacks JS Proxy, complex metaprogramming required); No built-in state management (rejected: consumers would each create ad-hoc solutions); Full MVVM framework (rejected: over-engineering for initial scope)
- **Reference**: React useState/useReducer pattern, unidirectional data flow

### Spec-kit Templates: YAML + Markdown

- **Decision**: Provide both YAML (structured) and Markdown (narrative) templates for feature specifications.
- **Rationale**: YAML suits structured widget specs (properties, methods, types). Markdown suits narrative architecture decisions and complex widget behavior descriptions.
- **Alternatives considered**: YAML-only (rejected: poor for narrative descriptions); Markdown-only (rejected: no structured data validation)

---

## Risk Assessment

| Risk | Mitigation | Status |
|------|------------|--------|
| Skia dependency leaks into non-render modules | CI visibility query enforces isolation | Active — query must be in CI |
| Inconsistent architecture interpretation across agents | Architecture docs + agent instructions | Active — docs not yet written |
| Widget lifecycle edge cases cause crashes | Lifecycle state machine documented + tests | Active — lifecycle model in this phase |
| Golden test flakiness (platform pixel differences) | Platform-specific baselines, tolerance config | Active — strategy in testing doc |
| Release process not followed | Automated release workflow | Active — release.yml to be created |
