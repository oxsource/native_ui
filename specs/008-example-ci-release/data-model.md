# Data / Entity Model: Example, CI Polish & Release

**Date**: 2026-07-30

## Entity: Hello World Example

| File | `examples/hello_world.cc` |
|------|--------------------------|
| **Type** | Self-contained cc_binary |
| **Deps** | `//src/framework/public:native_ui` |

**Pipeline**:
1. Create CounterState with `Property<int> count`
2. Build widget tree: Container(Direction(kColumn)) → Text (bound to count) + Button (OnClick increments count)
3. Run layout: `container->Measure(...)`, `container->Arrange(...)`
4. Create Skia Surface and Canvas
5. Draw: `container->Draw(canvas)`
6. Encode: `surface->Flush()`, write PNG to file
7. Click simulation: push MouseEvent via EventHub → State update → redraw
8. Write second PNG with updated state

## Entity: CI Pipeline

| Component | File | Purpose |
|-----------|------|---------|
| CI workflow | `.github/workflows/ci.yml` | `bazel build //...` + `bazel test //...` + visibility query on push |
| PR gate | `.github/workflows/pr.yml` | Same checks on PRs, mandatory review |
| Release workflow | `.github/workflows/release.yml` | Tag → build shared lib → create release |
| Visibility guard | inline in ci.yml | `bazel query 'somepath(//src/framework/..., @skia//:skia)'` |

## Entity: Shared Library

| Artifact | Platform | Path |
|----------|----------|------|
| macOS | `libnative_ui_shared.dylib` | `bazel-bin/src/framework/public/libnative_ui_shared.dylib` → `dist/` |
| Linux | `libnative_ui_shared.so` | `bazel-bin/src/framework/public/libnative_ui_shared.so` → `dist/` |

## Entity: Full Pipeline Integration Test

| File | `tests/integration/full_pipeline_test.cc` |
|------|------------------------------------------|
| **Scope** | Container → FlexLayout → Canvas draw → Skia surface → PNG encode |
| **Verification** | Pixel readback: verify a specific pixel has the expected color after rendering |

## Entity: CHANGELOG

| Section | Content |
|---------|---------|
| v0.1.0 (MVP) | Added: core types, widget system, layout engine, render wrappers, event system, example app |

## Relationships

```
Hello World Example
  ├── uses: public API headers (core, widgets, render, surface)
  ├── builds: Bazel cc_binary
  ├── runs: layout → render → encode pipeline
  └── verified by: examples_test.cc (smoke test)

CI Pipeline
  ├── ci.yml: triggers on push, builds + tests + visibility check
  ├── pr.yml: triggers on PR, same checks
  └── release.yml: triggers on tag, builds shared lib + GitHub Release

Release Process
  ├── build_shared.sh: builds shared library, copies to dist/
  ├── release.yml: creates GitHub Release with artifact
  └── CHANGELOG.md: documents version changes
```

## Validation Rules

| Rule | Entity | Description |
|------|--------|-------------|
| Example must exit 0 | Hello World | `bazel run //examples:hello_world` returns 0 |
| Example must produce PNG | Hello World | Output file exists and has valid PNG header |
| CI must pass both platforms | CI Pipeline | Build + test on macOS ARM64 and Linux x86_64 |
| Visibility guard must pass | CI Pipeline | `bazel query` returns only render/surface paths |
| Shared library must load | Shared Library | `dlopen`/`LoadLibrary` succeeds on the produced artifact |
