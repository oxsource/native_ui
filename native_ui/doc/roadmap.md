# Native UI - Development Roadmap

> Version: 0.1
>
> Status: Draft
>
> Purpose: Phased development plan with deliverables, dependencies, and agent task breakdown.

---

# Overview

The project is delivered in **10 phases** (8 delivery + 2 buffer), each building on the previous. Every phase produces a shippable increment — compilable, testable, and reviewable. Buffer phases exist to absorb risk and ensure quality before ship.

```
Phase 1 ──→ Phase 2 ──→ Phase 3 ──→ ... ──→ Phase 8 ──→ Buffer 1 ──→ Buffer 2 ──→ Ship
(Scaffold)   (Arch.)     (Core)                (Example)
            └── CI + Spike
```

| Phase | Theme | Depends On | Effort |
|-------|-------|------------|--------|
| P1 | Project Scaffolding & Skia Spike | — | M |
| P2 | Architecture & Engineering Design | P1 | M |
| P3 | Core Types & Widget Foundation | P1, P2 | M |
| P4 | Flexbox Layout Engine | P3 | M |
| P5 | Skia Render Wrapper & Platform Surface | P3 | M |
| P6 | Basic Widgets & Dynamic Tree | P3, P4, P5 | L |
| P7 | Event System & Observability | P3, P6 | M |
| P8 | Example, CI Polish & Release | P1–P7 | M |
| B1 | Buffer / Contingency | P8 | M |
| B2 | Buffer / Stabilization | B1 | M |

---

# Phase 1: Project Scaffolding & Skia Spike

**Goal**: Establish Bazel workspace, platform definitions, dependency management, and **validate Skia integration via a spike**. This phase has the highest technical risk — Skia's build system is complex and its Bazel integration often requires significant work. The spike ensures Skia compiles and links before any downstream code depends on it.

## Dependencies

- None (project bootstrap)

## Deliverables

### Build Infrastructure

| File | Purpose |
|------|---------|
| `WORKSPACE` | `workspace(name = "native_ui")` + `native_ui_setup()` call |
| `native_ui_deps.bzl` | External dep bootstrap: Skia, Yoga, googletest, skylib |
| `.bazelversion` | `6.5.0` |
| `.bazelrc` | C++17, visibility=hidden, platform aliases |
| `.bazelignore` | Ignore example workspaces |
| `BUILD.bazel` | Root alias `//:native_ui` → `//src/framework/public:native_ui` |
| `platforms/BUILD` | `config_setting` + `platform` for macos_arm64, linux_x86_64 |
| `platforms/platforms.bzl` | Helper macros |
| `third_party/skia/BUILD.bazel` | Skia `cc_library` wrapper |
| `third_party/yoga/BUILD.bazel` | Yoga `cc_library` wrapper |

### Source Stubs

| File | Purpose |
|------|---------|
| `src/framework/core/BUILD.bazel` | Empty `cc_library` target |
| `src/framework/layout/BUILD.bazel` | Empty `cc_library` target |
| `src/framework/render/BUILD.bazel` | Empty `cc_library` target |
| `src/framework/surface/BUILD.bazel` | Empty `cc_library` target |
| `src/framework/widgets/BUILD.bazel` | Empty `cc_library` target |
| `src/framework/event/BUILD.bazel` | Empty `cc_library` target |
| `src/framework/public/BUILD.bazel` | Umbrella target aggregating all modules |
| `src/framework/public/include/native_ui/native_ui_export.h` | `NATIVE_UI_API` macro |

### Skia Integration Spike

| File | Purpose |
|------|---------|
| `src/spike/skia_spike.cc` | Minimal binary: `#include "SkCanvas.h"`, create surface, draw red rect, write PNG |
| `src/spike/BUILD.bazel` | `cc_binary` depending on `@skia//:skia` |

The spike validates:
- Skia headers resolve correctly via `strip_include_prefix`
- Skia source files compile without errors
- Platform-specific linkopts (CoreGraphics, Metal, etc.) link correctly
- A simple draw + encode pipeline works end-to-end

If the spike fails, **stop and fix Skia integration before proceeding**. Do not enter P2 until the spike passes on the target platform.

### Spec Files

| File | Purpose |
|------|---------|
| `spec/native_ui/build.yaml` | Build convention spec for agents |

### Acceptance Criteria

- `bazel build //src/spike:skia_spike` succeeds
- `bazel run //src/spike:skia_spike` produces a valid PNG output
- `bazel build //...` succeeds (all targets green)
- `bazel test //...` passes (no tests yet, but test infra exists)
- Root alias `//:native_ui` resolves correctly

---

# Phase 2: Architecture & Engineering Design

**Goal**: Establish architectural foundations — module interface contracts, error handling, memory model, widget lifecycle, **data binding (React-inspired `State`)**, **threading model (main-thread rendering + worker-thread logic)**, **logging slot interface (LogSlot)**, testing conventions, CI/CD pipeline, release process, and spec-kit templates. This phase produces **design artifacts only** — no runtime code.

## Dependencies

- P1 (build system ready to compile stubs, Skia Spike + Yoga Spike pass)

## Deliverables

### Architecture Design Records

| File | Purpose |
|------|---------|
| `doc/architecture/README.md` | Architecture overview, key decisions index |
| `doc/architecture/module_dependencies.md` | Formal module dependency graph with visibility rules |
| `doc/architecture/error_handling.md` | Error propagation strategy (StatusOr, exceptions policy) |
| `doc/architecture/memory_model.md` | Ownership model, WidgetPtr vs raw pointer conventions |
| `doc/architecture/lifecycle_model.md` | Widget lifecycle state machine, mount/unmount semantics |
| `doc/architecture/data_binding.md` | `State` + `Property<T>` pattern, typed `Watch(Property<T>&)`, extension hooks (`OnBeforeSet`, `OnAfterSet`), batch RequestRedraw |
| `doc/architecture/threading.md` | Threading model: main thread (render/layout/event) + worker threads (logic/data), inter-thread communication via State |
| `doc/architecture/logging_slot.md` | LogSlot abstract interface (debug/info/warn/error), slot pattern, consumer-plugged implementation |

### API & Interface Contracts

| File | Purpose |
|------|---------|
| `doc/api/widget_contract.md` | Widget virtual interface, extension points, custom widget guide |
| `doc/api/layout_contract.md` | FlexLayout interface, measure/arrange protocol, adding new layouts |
| `doc/api/render_contract.md` | Canvas/Paint/Path contract, Skia isolation rules |
| `doc/api/event_contract.md` | Event dispatch protocol, bubble/capture, adding event types |
| `doc/api/viewmodel_contract.md` | `State` + `Property<T>` template, `Property<T>::operator=`, `Watch(Property<T>&)`, extension hooks, thread safety |

### Engineering Standards

| File | Purpose |
|------|---------|
| `doc/testing_strategy.md` | Unit test structure, mock patterns, integration test scopes, golden test plan, coverage targets |
| `doc/build_conventions.md` | BUILD file conventions, dep prefix rules, visibility template |
| `doc/agent_instructions.md` | Standard prompt template for opencode agents working on native_ui |

### CI/CD Pipeline

| File | Purpose |
|------|---------|
| `.github/workflows/ci.yml` | CI pipeline: `bazel build //...`, `bazel test //...`, clang-format, clang-tidy |
| `.github/workflows/pr.yml` | PR gate: same checks + mandatory review approval |
| `.github/workflows/release.yml` | Release workflow: tag → build shared lib → create GitHub Release |
| `doc/ci_strategy.md` | CI architecture doc: which checks run when, caching strategy, matrix platforms |

CI checks at minimum:
- `bazel build //...` (all targets compile)
- `bazel test //...` (all tests pass)
- `clang-format --dry-run --Werror` (code formatting)
- `clang-tidy` (lint, only on changed files for speed)
- Bazel visibility query: verify no module outside `render/` depends on `@skia`

### Release Engineering

| File | Purpose |
|------|---------|
| `doc/release_process.md` | Release workflow: versioning (SemVer), CHANGELOG.md convention, shared lib publishing |
| `CHANGELOG.md` | Placeholder, populated on each release |

### Spec-kit Templates

| File | Purpose |
|------|---------|
| `spec/native_ui/_template.yaml` | spec-kit YAML template with all sections (interface, behavior, bounds, tests) |
| `spec/native_ui/_template_layout.md` | Alternative markdown template for complex widget specs |

### Source Stubs

| File | Purpose |
|------|---------|
| `src/framework/viewmodel/BUILD.bazel` | Empty `cc_library` stub for future State module |

### Acceptance Criteria

- All design documents pass internal review (no contradictions with project_bootstrap.md)
- Module dependency graph is consistent with Bazel visibility rules in `build_conventions.md`
- CI pipeline runs successfully on a test push (all checks green)
- Skia isolation query (`bazel query 'somepath(//src/framework/..., @skia//:skia)'`) validates only `render/` + `surface/` depend on Skia
- spec-kit template covers: interface signature, behavior description, edge cases, test points
- Agent instruction template includes Google C++ Style checklist and commit convention reminder
- Architecture docs cover: threading model (main vs worker), logging slot (LogSlot), data binding (`State`)
- No C++ source code (other than stubs) is written in this phase

---

# Phase 3: Core Types & Widget Foundation + State

**Goal**: Implement foundational geometry and color types, the `Widget` base class, and the **`State`** base class with property notification. Establish the core data binding infrastructure.

## Dependencies

- P1 (workspace compiles)
- P2 (architecture contracts defined)

## Deliverables

### Source Files

| File | Content |
|------|---------|
| `src/framework/core/rect.h / rect.cc` | `Rect` — position, size, containment, intersection |
| `src/framework/core/point.h / point.cc` | `Point` — x/y |
| `src/framework/core/size.h / size.cc` | `Size` — width/height |
| `src/framework/core/color.h / color.cc` | `Color` — RGBA, named colors (`kRed`, `kBlue`, etc.) |
| `src/framework/core/edge_insets.h / edge_insets.cc` | `EdgeInsets` — symmetric, per-side |
| `src/framework/viewmodel/state.h / state.cc` | `State` base + `Property<T>` template — property change notification via `operator=`, thread-safe update, `Watch(Property<T>&)`, extension hooks |
| `src/framework/widgets/widget.h / widget.cc` | `Widget` base — `SetId`, `FindById`, `RequestLayout`, `RequestRedraw`, `ChildAt`, `ChildCount` |
| `src/framework/widgets/container.h / container.cc` | `Container` — tagged-parameter ctor, `AddChild`, `RemoveChild`, `ClearChildren` |

### Public Headers

| File | Content |
|------|---------|
| `src/framework/public/include/native_ui/core.h` | Re-export core types |
| `src/framework/public/include/native_ui/widgets.h` | Re-export Widget + Container + State |

### Tests

| File | Content |
|------|---------|
| `tests/core_test.cc` | Rect, Point, Size, Color, EdgeInsets unit tests |
| `tests/state_test.cc` | State property notification, cross-thread update, Watch/Unwatch lifecycle |
| `tests/widget_test.cc` | Widget ID, FindById, Container add/remove/clear |
| `tests/integration/container_layout_test.cc` | Cross-module: Container + FlexLayout + core types end-to-end |

### Spec Files

| File | Purpose |
|------|---------|
| `spec/native_ui/core.yaml` | Core type specs |
| `spec/native_ui/state.yaml` | State base class spec |
| `spec/native_ui/widget_base.yaml` | Widget base class spec |

### Acceptance Criteria

- `Rect::Contains`, `Rect::Intersect`, `Color` blending work correctly
- `State` property change triggers notification to watching widgets
- `Container(Direction(kRow), Padding(16), Children{...})` compiles
- `FindById("x")` returns correct `Widget*`
- `AddChild` / `RemoveChildAt` / `ClearChildren` trigger `RequestLayout`
- Integration test covers Container → FlexLayout pipeline
- `bazel test //tests:core_test` green
- `bazel test //tests:viewmodel_test` green
- `bazel test //tests:widget_test` green
- `bazel test //tests:integration:container_layout_test` green

---

# Phase 4: Flexbox Layout Engine

**Goal**: Wrap Yoga into `FlexLayout` with tagged-parameter API. Implement measure + arrange pipeline.

## Dependencies

- P3 (Core Types & Widget Foundation)

## Deliverables

### Source Files

| File | Content |
|------|---------|
| `src/framework/layout/flex_layout.h / flex_layout.cc` | `FlexLayout` — tagged ctor, `Measure`, `Arrange` |
| `src/framework/layout/layout_result.h` | `MeasureResult`, `ArrangeResult` structs |

### Public Headers

| File | Content |
|------|---------|
| `src/framework/public/include/native_ui/layout.h` | Re-export FlexLayout |

### Tests

| File | Content |
|------|---------|
| `tests/layout_test.cc` | Direction, justify, align, wrap, gap measure/arrange |

### Spec Files

| File | Purpose |
|------|---------|
| `spec/native_ui/flex_layout.yaml` | FlexLayout API and behavior spec |

### Acceptance Criteria

- `FlexLayout(Direction(kRow), Gap(8), Padding(12))` compiles
- `Measure({800, 600})` returns correct child sizes for row/column
- `Arrange(...)` positions children at expected offsets
- Supports `kRow`, `kColumn`, wrapping, grow/shrink basis
- `bazel test //tests:layout_test` green

---

# Phase 5: Skia Render Wrapper & Platform Surface

**Goal**: Build `Surface`, `Canvas`, `Paint`, `Path` RAII wrappers over Skia, plus `Surface` for platform-native buffer rendering (AHardwareBuffer / IOSurface / DMA-BUF). These are the only modules that depend on Skia directly.

> **Scope note**: `TextLayout` is intentionally deferred to a post-MVP phase. Skia's text stack (SkParagraph/SkShaper) has complex dependencies (ICU, harfbuzz) that increase integration risk. Initial widget rendering uses simple `DrawText` until `TextLayout` lands.

## Dependencies

- P3 (Core Types: Rect, Point, Size, Color)

## Deliverables

> **Key differentiator**: This phase also lays the foundation for platform native Buffer support (AHardwareBuffer / IOSurface / DMA-BUF). The `surface/` module provides `Surface` — a composable wrapper over Skia's `SkSurface` for both display and external buffer rendering, enabling zero-copy GPU-side rendering for camera preview, video decode, and AI inference overlay.

### Source Files

| File | Content |
|------|---------|
| `src/framework/render/canvas.h / canvas.cc` | Scoped `Canvas` — attach to `Surface&`, auto save/restore, primitives (`DrawRect`, `DrawText`, `DrawPath`), image drawing (`DrawImage` with Image, BufferHandle) |
| `src/framework/render/paint.h / paint.cc` | `Paint` — chainable `SetColor`, `SetAntiAlias`, `SetStrokeWidth` |
| `src/framework/render/path.h / path.cc` | `Path` — `MoveTo`, `LineTo`, `CubicTo`, `Close` |
| `src/framework/render/image.h / image.cc` | `Image` — decode from PNG/JPEG/WebP/SVG, wrap platform Buffer (AHardwareBuffer / IOSurface / DMA-BUF), `FromEncoded`, `FromFile`, `FromBuffer`, `FromSvg` |
| `src/framework/surface/surface.h / surface.cc` | `Surface` — composable wrapper over SkSurface, supports display and external buffer rendering |
| `src/framework/surface/buffer_handle.h` | `BufferHandle` — type-erased cross-platform buffer descriptor (AHardwareBuffer / IOSurface / DMA-BUF fd) |
| `src/framework/surface/surface_factory.h / surface_factory.cc` | `SurfaceFactory` — platform dispatch via `#ifdef`, creates platform-specific SkSurface |

### Public Headers

| File | Content |
|------|---------|
| `src/framework/public/include/native_ui/render.h` | Re-export render types |
| `src/framework/public/include/native_ui/surface.h` | Re-export Surface, BufferHandle |

### Tests

| File | Content |
|------|---------|
| `tests/render_test.cc` | Canvas save/restore, Paint chain, Path construction, pixel readback verification |
| `tests/golden/skia_spike_test.cc` | Golden baseline: render known rect → compare PNG hash against committed baseline |
| `tests/surface_test.cc` | Surface creation from synthetic buffer, buffer update callback, surface lifecycle |

Golden test flow:
1. Render a known scene to a Skia surface
2. Encode as PNG to `tests/golden/baseline/`
3. On subsequent runs, compare output hash against baseline
4. CI fails if hash diverges (detects rendering regressions)

### Spec Files

| File | Purpose |
|------|---------|
| `spec/native_ui/render_canvas.yaml` | Canvas spec |
| `spec/native_ui/render_paint.yaml` | Paint spec |
| `spec/native_ui/render_image.yaml` | Image spec (FromEncoded, FromBuffer, FromSvg, DrawImage) |

### Acceptance Criteria

- `Canvas::DrawRect(Rect{0,0,100,50}, Paint().SetColor(Color::kRed))` draws correctly
- `Canvas` auto-restore on scope exit
- `Paint` method chaining works
- Golden test produces identical PNG on repeat runs
- `Surface` accepts `BufferHandle` and creates a valid `SkSurface`
- Buffer update callback triggers correct `RequestLayout` / `RequestRedraw`
- **No module outside `render/` or `surface/` depends on Skia directly** (enforced by CI visibility query)
- `bazel test //tests:render_test` green
- `bazel test //tests:surface_test` green

---

# Phase 6: Basic Widgets & Dynamic Tree + Data Binding Integration

**Goal**: Implement `Text`, `Button`, `Image`, `Stack` widgets with full tagged-parameter construction, draw, layout invalidation, dynamic tree manipulation, and **data binding** (Watch State from Widget).

## Dependencies

- P3 (Core Types & Widget Foundation + State)
- P4 (Flexbox Layout Engine)
- P5 (Skia Render Wrapper)

## Deliverables

### Source Files

| File | Content |
|------|---------|
| `src/framework/widgets/text.h / text.cc` | `Text` — `Content`, font size, color, `Draw`, **Watch State** |
| `src/framework/widgets/button.h / button.cc` | `Button` — `Label`, click callback, hit area, **State event binding** |
| `src/framework/widgets/image.h / image.cc` | `Image` — `ImagePath`, Skia image decode + draw |
| `src/framework/widgets/stack.h / stack.cc` | `Stack` — layer-based child positioning |

### Tests

| File | Content |
|------|---------|
| `tests/widgets_test.cc` | Text layout + draw, Button hit area, Image load + draw, Stack layering, **State→Widget redraw** |

### Spec Files

| File | Purpose |
|------|---------|
| `spec/native_ui/widgets_text.yaml` | Text widget spec |
| `spec/native_ui/widgets_button.yaml` | Button widget spec |
| `spec/native_ui/widgets_image.yaml` | Image widget spec |
| `spec/native_ui/widgets_stack.yaml` | Stack widget spec |

### Acceptance Criteria

- `Text(Content("Hi"))` renders with correct size
- `Button(Label("OK"))` draws a clickable area
- `Image(ImagePath("/path.png"))` decodes and renders
- `Stack` renders children in layer order
- All widgets support `Id(...)` for `FindById` lookup
- **Widget watching State redraws automatically when State property changes**
- Dynamic `AddChild` / `RemoveChild` triggers re-layout on any Container widget
- `bazel test //tests:widgets_test` green

---

# Phase 7: Event System & Observability

**Goal**: Implement hit testing, event dispatch, mouse/touch input adapters, and a lightweight `DebugOverlay` for visual debugging.

## Dependencies

- P3 (Core Types: Point, Rect)
- P6 (Basic Widgets & Dynamic Tree)

## Deliverables

### Source Files

| File | Content |
|------|---------|
| `src/framework/event/event.h / event.cc` | `Event` types: `MouseEvent`, `KeyEvent`, `TouchEvent` |
| `src/framework/event/hit_tester.h / hit_tester.cc` | `HitTester` — DFS hit test on widget tree |
| `src/framework/widgets/debug_overlay.h / debug_overlay.cc` | `DebugOverlay` — toggleable overlay showing layout borders, FPS, widget tree breadcrumb |

### DebugOverlay

`DebugOverlay` is a **special widget** for development diagnostics:

```
DebugOverlay(Direction(kColumn), Children{
    Text(Content("FPS: 60")),
    Text(Content("Widgets: 12")),
    Divider(),
    Text(Content("[debug] Widget:root > Container#header > Text#title")),
})
```

Features:
- Toggle visibility with a key shortcut (e.g. F12 or Ctrl+D)
- Layout border overlay: draw colored outlines around all widget bounds
- FPS counter updated each frame
- Widget tree breadcrumb showing the branch under cursor
- **Compiled out** in release builds via `#ifndef NDEBUG`

### Public Headers

| File | Content |
|------|---------|
| `src/framework/public/include/native_ui/event.h` | Re-export event types |
| `src/framework/public/include/native_ui/debug_overlay.h` | Re-export DebugOverlay |

### Tests

| File | Content |
|------|---------|
| `tests/event_test.cc` | HitTest accuracy, event dispatch chain |
| `tests/debug_overlay_test.cc` | DebugOverlay toggling, border paint correctness |

### Spec Files

| File | Purpose |
|------|---------|
| `spec/native_ui/event.yaml` | Event system spec |
| `spec/native_ui/debug_overlay.yaml` | DebugOverlay spec |

### Acceptance Criteria

- `HitTester::Test(Point{10, 20})` returns correct widget
- Events bubble from leaf to root
- Mouse event carries position, button state
- Key event carries key code
- `DebugOverlay` toggleable, shows FPS + layout borders
- `DebugOverlay` excluded from release builds
- `bazel test //tests:event_test` green
- `bazel test //tests:debug_overlay_test` green

---

# Phase 8: Example, CI Polish & Release

**Goal**: End-to-end Hello World example (with **data binding** demonstrating State + Widget + Skia rendering), harden CI pipeline, finalize release process, and produce the first shared library artifact.

## Dependencies

- P1–P7 (all modules)

## Deliverables

### Source Files

| File | Content |
|------|---------|
| `examples/hello_world.cc` | Full MVP: build widget tree with State data binding, run layout, render to Skia surface, handle click → State update → auto redraw |

### Tests

| File | Content |
|------|---------|
| `tests/examples_test.cc` | Example smoke test: run hello_world and verify exit code 0 |
| `tests/integration/full_pipeline_test.cc` | Cross-module: Container → FlexLayout → Canvas draw → Skia surface encode |

### Release Engineering

| File | Purpose |
|------|---------|
| `CHANGELOG.md` | Populated with initial MVP changelog entry |
| `scripts/build_shared.sh` | Script: `bazel build //src/framework/public:native_ui_shared` + copy to `dist/` |
| `.github/workflows/release.yml` | Release workflow: build shared lib, create GH release, attach artifact |

### CI Hardening

- Verify CI pipeline passes on **both** macOS ARM64 and Linux x86_64 (matrix build)
- Verify `bazel query` visibility guard is part of CI
- Add CI caching for Bazel remote cache (if available)
- Set up CI status badge in README

### Acceptance Criteria

- `examples/hello_world.cc` compiles and runs
- Output shows correctly laid out and rendered widgets
- Click/tap on a Button triggers its callback
- `bazel build //...` succeeds on both macos_arm64 and linux_x86_64
- `bazel test //...` passes (all 15+ tests)
- Shared library `libnative_ui_shared.dylib` / `.so` builds successfully
- `CHANGELOG.md` documents MVP changes
- CI pipeline reports green with caching enabled

---

# Buffer & Contingency

Two buffer phases are allocated between the last delivery phase (P8) and the MVP ship decision. These absorb risk from:

- Skia integration issues discovered during P5
- Cross-platform compilation fixes (Linux CI matrix)
- Test failures requiring non-trivial refactoring
- Documentation gaps found during integration review
- Performance or correctness issues in golden tests

**B1** and **B2** are **optional** — if P1–P8 deliver on time and all acceptance criteria pass, these phases are dropped and the project moves directly to ship.

---

# Summary Timeline

```
P1 ────────────────────────────────────── ███   (Scaffold + Skia Spike)
P2 ────────────────────────────────────── ████  (Architecture + CI)
P3 ────────────────────────────────────── ████  (Core)
P4 ────────────────────────────────────── ████  (Layout)
P5 ────────────────────────────────────── █████ (Render + Golden)
P6 ────────────────────────────────────── ███████ (Widgets)
P7 ────────────────────────────────────── ████  (Event + DebugOverlay)
P8 ────────────────────────────────────── ███   (Example + Release)
B1 ────────────────────────────────────── ███   (Buffer / Contingency)
B2 ────────────────────────────────────── ██    (Buffer / Stabilization)

W1  W2  W3  W4  W5  W6  W7  W8  W9  W10 W11 W12 W13 W14 W15 W16
```

| Week | Phase | Major Milestone |
|------|-------|-----------------|
| 1–2 | P1 | `bazel build //...` green + Skia Spike passes |
| 3–4 | P2 | Architecture docs + CI pipeline green + spec templates |
| 5–6 | P3 | Core types + Widget base + Container + integration test |
| 7–8 | P4 | FlexLayout measure + arrange |
| 9–10 | P5 | Canvas + Paint + Path wrappers + golden test |
| 11–13 | P6 | Text, Button, Image, Stack widgets |
| 14 | P7 | Hit testing + event dispatch + DebugOverlay |
| 15 | P8 | Hello World example + CI hardening + shared lib |
| 16–17 | B1 | Contingency (risk absorb) |
| 18 | B2 | Final stabilization |
| — | **Ship** | **MVP v0.1.0 released** |

---

# Risk Register

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| Skia build integration failure | Blocks P5–P8 | High | Spike in P1; stop-gate before P2 |
| Skia API surface changes | Render wrapper rewrite | Low | Pin specific Skia commit |
| Yoga API changes | Layout engine rework | Low | Yoga is well-established with stable API |
| Cross-platform Skia linkopts | CI red on Linux | Medium | Spike tests both platforms in CI |
| TextLayout complexity | Feature creep | High (deferred) | Explicitly deferred post-MVP |
| Performance (full re-layout on every add) | UI jank | Low | RequestLayout batching planned |
| State thread safety | Data race on property update | Medium | Document thread boundaries in threading.md; State update must be thread-safe |
| Logging slot not plugged | Silent failures | Low | Document fallback behavior (no-op when no LogSlot) |
| CI maintenance overhead | Engineer velocity | Low | Shared Bazel cache, minimal workflow |

---

# Future Phases (Post-MVP)

| Phase | Theme | Notes |
|-------|-------|-------|
| P9  | TextLayout | SkParagraph integration, font loading, text alignment |
| P10 | Platform Buffer Hardening | AHardwareBuffer / IOSurface / DMA-BUF per-platform production hardening, zero-copy benchmarks |
| P11 | Animation System | Keyframe, tween, transition |
| P12 | Complex Widgets | ListView, TreeView, Table |
| P13 | RTL / I18n | Bidirectional layout, text direction |
| P14 | GPU Acceleration | Skia GPU backend, Metal/Vulkan |
| P15 | Embedded Platform | Linux ARM64, framebuffer |
| P16 | Visual Designer | spec-kit → widget tree codegen |
