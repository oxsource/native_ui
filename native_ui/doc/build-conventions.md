# Build Conventions

**Last Updated**: 2026-07-29

## BUILD File Template

```python
# src/framework/<module>/BUILD.bazel
cc_library(
    name = "<module>",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    deps = [
        "//src/framework/core",          # internal deps use //src/framework/
        "@yoga//:yoga",                  # external deps use @
    ],
    visibility = [
        "//src/framework:__subpackages__",  # internal visibility
        "//tests:__subpackages__",          # test visibility
    ],
)
```

## Dependency Prefix Rules

| Prefix | Meaning | Example |
|--------|---------|---------|
| `//src/framework/` | Internal module dependency | `//src/framework/core` |
| `//tests/` | Test dependency | `//tests:infra_test` |
| `@` | External third-party | `@skia//:skia`, `@yoga//:yoga` |

## Visibility Rules

| Target | Visibility | Can Depend On |
|--------|------------|---------------|
| `//src/framework/public:native_ui` | `//visibility:public` | Any project |
| Internal module (`core`, `layout`, etc.) | `__subpackages__` + `tests` | Same framework, tests only |
| `@skia//:skia` | `//visibility:public` | Only render/ and surface/ |

## Skia Isolation Enforcement

```python
# In .github/workflows/ci.yml — Bazel visibility query
bazel query 'somepath(//src/framework/..., @skia//:skia)' \
  | grep -v '//src/framework/render' | grep -v '//src/framework/surface'
# If any path remains → CI fails
```

## Naming

- BUILD file: `BUILD.bazel` (not `BUILD`)
- Library target: lowercase module name
- Binary target: `snake_case` (e.g., `skia_spike`)
- Test target: `*_test` suffix
