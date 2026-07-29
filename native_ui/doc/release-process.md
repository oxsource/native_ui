# Release Process

**Last Updated**: 2026-07-29

## Versioning

SemVer: `MAJOR.MINOR.PATCH`

| Increment | When |
|-----------|------|
| MAJOR | Breaking API change | 
| MINOR | New feature, backward compatible |
| PATCH | Bug fix, no API change |

## Release Workflow

```text
1. Update CHANGELOG.md   (move [Unreleased] → new version section)
2. Tag release           git tag -a v0.1.0 -m "v0.1.0"
3. Push tag              git push origin v0.1.0
                         (triggers .github/workflows/release.yml)
4. Release workflow:
   ├─ bazel build //src/framework/public:native_ui_shared
   ├─ Create GitHub Release
   └─ Attach .dylib + .so artifacts
```

## Hotfix Strategy

```text
v0.1.0 ─── v0.1.1 (hotfix)
   │           ↑
   └─── hotfix/v0.1.x ─── cherry-pick fix
```

- Hotfix branch from release tag: `git checkout -b hotfix/v0.1.x v0.1.0`
- Commit fix, tag `v0.1.1`, push tag
- Merge back to main

## Building Shared Library

```bash
# macOS
bazel build //src/framework/public:native_ui_shared
# output: bazel-bin/src/framework/public/libnative_ui_shared.dylib

# Linux
bazel build //src/framework/public:native_ui_shared
# output: bazel-bin/src/framework/public/libnative_ui_shared.so
```
