# Agent Instructions for opencode

**Last Updated**: 2026-07-29

## Standard Prompt Template

When working on native_ui, each agent must:

1. Read the relevant spec files from `specs/` for interface contracts and requirements
2. Read architecture docs from `native_ui/doc/architecture/` for design context
3. Follow Google C++ Style Guide (2-space indent, 80-col width, snake_case files, PascalCase types)
4. Tagged-parameter constructor pattern for all concrete widgets
5. `namespace native::ui { }` for all framework code
6. No C++ exceptions — use `StatusOr` for recoverable errors
7. Build + test before submitting

## Commands

```bash
# Build everything
bazel build //...

# Build specific module
bazel build //src/framework/core

# Run all tests
bazel test //...

# Run specific test
bazel test //tests:infra_test --test_output=all

# Run spike binary
bazel run //src/spike:skia_spike -- /tmp/output.png
```

## Commit Convention

```
<type>(<scope>): <description>

feat(core): add Rect base type
feat(layout): implement Flexbox measure/arrange
docs(arch): add threading model
```

Types: feat, fix, docs, refactor, test, build
Scopes: core, layout, render, surface, viewmodel, widgets, event, public, spike, docs, build
