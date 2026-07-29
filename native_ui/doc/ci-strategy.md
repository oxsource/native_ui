# CI Strategy

**Last Updated**: 2026-07-29

## Pipeline Overview

```text
Push / PR
   │
   ▼
┌─────────────────────────────┐
│ 1. bazel build //...        │  ~3-5 min (cached)
├─────────────────────────────┤
│ 2. bazel test //...         │  ~1-2 min
├─────────────────────────────┤
│ 3. clang-format --dry-run   │  ~30s
├─────────────────────────────┤
│ 4. clang-tidy (changed)     │  ~2 min
├─────────────────────────────┤
│ 5. Bazel visibility query   │  ~10s
├─────────────────────────────┤
│ 6. ✅ / ❌                 │
└─────────────────────────────┘
```

## Check Sequence

| Step | Command | Expected Time | Fail Fast? |
|------|---------|---------------|------------|
| Build | `bazel build //...` | 3-5 min (cold: 30+ min) | Yes |
| Test | `bazel test //...` | 1-2 min | Yes |
| Format | `clang-format --dry-run --Werror` | 30s | Yes |
| Lint | `clang-tidy` (changed files only) | 2 min | No (warnings) |
| Isolation | `bazel query 'somepath(//src/framework/..., @skia//:skia)'` | 10s | Yes |

## Caching Strategy

- Bazel remote cache (if available) — share build outputs across CI runs
- `.bazelrc`: `build --disk_cache=~/.cache/bazel-disk` for local caching
- CI cache key: `bazel-{{ .Workflow }}-{{ .Platform }}` (platform-specific)

## Matrix

| Platform | Runs On | Status |
|----------|---------|--------|
| macOS ARM64 | GitHub macOS runner | Required (primary dev) |
| Linux x86_64 | GitHub ubuntu runner | Required (CI + release) |

## Visibility Query Check

```bash
# Must return zero paths outside render/ + surface/
bazel query 'somepath(//src/framework/..., @skia//:skia)' \
  | grep -v '//src/framework/render' | grep -v '//src/framework/surface'
# Fail if output is non-empty
```
