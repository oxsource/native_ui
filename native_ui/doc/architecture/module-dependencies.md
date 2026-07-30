# Module Dependencies & Bazel Visibility

**Last Updated**: 2026-07-29

## Module Dependency Graph

```
                       ┌──────────┐
                       │  public  │
                       │ (facade) │
                       └────┬─────┘
                            │
              ┌─────────────┼─────────────┐
              │             │             │
        ┌─────▼────┐  ┌────▼─────┐  ┌────▼─────┐
        │ widgets  │  │ state│  │  event   │
        └─────┬────┘  └────┬─────┘  └────┬─────┘
              │             │             │
        ┌─────┼─────────────┼─────────────┘
        │     │             │
  ┌─────▼──┐ │  ┌──────────▼─────┐
  │ layout │ │  │    render      │
  │ (Yoga) │ │  │    (Skia)      │
  └────┬───┘ │  └────────┬───────┘
       │     │           │
       │  ┌──▼───────────▼────┐
       │  │      surface      │
       │  │    (Skia, plat)   │
       │  └────────┬──────────┘
       │           │
       └─────┬─────┘
             │
       ┌─────▼─────┐
       │   core    │
       │ (no deps) │
       └───────────┘
```

## Dependency Rules

| Module | Internal Dependencies | External Dependencies |
|--------|----------------------|----------------------|
| `core` | — | — |
| `layout` | core | Yoga |
| `render` | — | Skia |
| `surface` | — | Skia, platform headers |
| `state` | core | — |
| `widgets` | core, state, layout, render, event | — |
| `event` | core | — |
| `public` | core, layout, render, surface, state, widgets, event | — |

## Bazel Visibility Rules

All internal modules use restricted visibility:

```python
visibility = ["//src/framework:__subpackages__", "//tests:__subpackages__"]
```

Only `//src/framework/public:native_ui` has `//visibility:public`.

## Skia Isolation Policy

**No module outside `render/` and `surface/` may depend on `@skia//:skia`.**

This is enforced by CI via:

```bash
bazel query 'somepath(//src/framework/..., @skia//:skia)' \
  | grep -v '//src/framework/render' | grep -v '//src/framework/surface'
```

If this query returns any paths outside render/ or surface/, CI fails.

## Adding a New Module

1. Create the module directory under `src/framework/<name>/`
2. Create `BUILD.bazel` with `cc_library` and restricted visibility
3. Add to the `public` target's `deps` if the module should be exposed externally
4. Run `bazel query` to verify no Skia dependency leaks
