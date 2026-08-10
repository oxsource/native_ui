# Verification Tooling Plan: Categorized Scripts + Makefile Mechanism

**Status**: Proposed
**Last Updated**: 2026-08-10
**Scope**: `native_ui/scripts/` + a root `Makefile` for build/verify/tool tasks.

## 1. Motivation

Developer-facing build/verification tasks are currently ad-hoc:

- `scripts/build_shared.sh` — build the shared library.
- `scripts/fetch_nanosvg.sh` — obsolete reference (nanosvg is now a Bazel `http_archive`).
- `scripts/android_external_image.sh` — Android demo build/push/run (device verification loop).

There is no shared helper code, no consistent interface, and no single entry point, so each
script re-implements NDK detection, adb handling, and logging. This plan introduces:

1. a **categorization convention** for scripts (flat `prefix_` naming),
2. a **shared library** (`scripts/lib/common.sh`, `scripts/lib/pixel_diff.py`),
3. a **Makefile** as the primary, discoverable mechanism,
4. a **decode-back pixel-diff** step that closes the Android device verification loop (feature
   `011-ahwb-external-image`, task T033).

## 2. Design

### 2.1 Categorization convention

Verification scripts live in a **dedicated subdirectory** `scripts/verify/`; build/tool scripts
stay in `scripts/`:

| Location | Purpose | Files |
|----------|---------|-------|
| `scripts/` | Build artifacts + misc tools | `build_shared.sh`, `tool_fetch_nanosvg.sh` |
| `scripts/verify/` | Verification flows | `host.sh`, `android_build.sh`, `android_demo.sh` |
| `scripts/lib/` | Shared helpers (sourced by scripts) | `common.sh`, `pixel_diff.py` |

Rationale: keeping all verification in one directory makes the surface discoverable (`make`,
the Makefile, or `ls scripts/verify`) and separates "does it build" (build) from "is it
correct" (verify).

### 2.2 Shared library

`scripts/lib/common.sh` (sourced by every script):

- Logging: `log_info`, `log_warn`, `log_error`, `log_ok` (colorized when stdout is a TTY), `die`.
- `resolve_root` — the Bazel workspace root (parent of `scripts/`).
- `bazel_build <target...> [--config NAME]` — runs `bazel build` in the workspace root.
- `require_ndk` — validates `$ANDROID_NDK_HOME` or `--ndk` (NDK >= r25b), prints
  `source.properties` revision, dies with a clear message otherwise.
- `detect_adb` / `adb_push` / `adb_pull` — respect `$ANDROID_SERIAL` or `--device`.
- Defaults: device temp dir `/data/local/tmp`, host artifact dir `<root>/out`.

`scripts/lib/pixel_diff.py` (stdlib only): compares two raw RGB frames (e.g., MP4 frame 0 vs the
demo's CPU snapshot) and prints max/mean absolute difference; exits non-zero when the mean
difference exceeds a small threshold (tolerating H.264 lossy encoding).

### 2.3 Scripts

| Script | Behaviour |
|--------|-----------|
| `scripts/verify/host.sh` | `bazel test //tests/... //tests/golden/... //tests/integration/...` (host CI, feature T032) |
| `scripts/verify/android_build.sh` | `require_ndk` + `bazel build --config=android_arm64` of `surface`/`render`/`widgets` + `external_image_demo`; sanity-check the produced aarch64 ELF |
| `scripts/verify/android_demo.sh` | Device closed loop for `//examples:external_image_demo` (moved + enhanced from `android_external_image.sh`) |
| `scripts/build_shared.sh` | Unchanged (build the shared library) |
| `scripts/tool_fetch_nanosvg.sh` | Renamed reference; nanosvg is fetched via `native_ui_deps.bzl` |

`scripts/verify/android_demo.sh` subcommands:

| Subcommand | Behaviour |
|------------|-----------|
| `build` | `require_ndk` + cross-compile the demo |
| `push` | adb push binary + source PNG to `/data/local/tmp` |
| `run` | adb run (optional `--live[=SECONDS]`), then pull MP4 + PNG artifacts to `out/` |
| `pull` | Pull the last run's artifacts (MP4 + PNGs) into `out/` |
| `list` | `ls -l` the demo artifacts on the device |
| `diff` | Decode-back + pixel-diff: ffmpeg extracts MP4 frame 0 → raw RGB; ffmpeg decodes `external_image_cpu.png` → raw RGB; `pixel_diff.py` compares (feature T033) |
| `view` | Open the pulled MP4 + PNGs for visual comparison (`open`, macOS) |
| `all` | `build` + `push` + `run` + `pull` + `diff` in one go |

`--live[=SECONDS]` runs the demo's live mode (30 Hz cycling buffers), which prints per-frame
avg/max time and VmRSS before/after (feature T034 data).

### 2.4 Makefile — primary mechanism

A root `Makefile` in the Bazel workspace (`native_ui/`) is the single discoverable
interface. Targets are **modularized** — each category is a separate `mk/<category>.mk`
file auto-included by the root Makefile (`include $(filter-out mk/rules.mk,$(wildcard mk/*.mk))`).

**AOSP-style module registry** (`mk/rules.mk`): each module self-describes, mirroring
`LOCAL_MODULE`:

```make
$(call register_module, <name>)      # unique module name — duplicates abort the build
$(call register_target,  <target>)   # every target it owns — duplicates abort the build
$(call register_alias,   <alias>, <target>)  # friendly short name — duplicates abort
```

**Canonical targets are namespaced `<module>-<action>`** (e.g. `android-build`,
`host-verify`) so modules can never clash; friendly short aliases live in
`mk/aliases.mk`. Adding a new category is just a new `mk/<name>.mk`.

| Module | Contents |
|--------|----------|
| `mk/rules.mk` | Core registry (`register_module` / `register_target` / `register_alias`) |
| `mk/help.mk` | `help`, `modules` |
| `mk/host.mk` | Host verification (`host-verify`) |
| `mk/android.mk` | Android build + device flow (`android-*` targets) |
| `mk/aliases.mk` | Friendly aliases (`verify`, `build-android`, `demo`, `pull`, `list`, `diff`, `view`, `results`, `demo-live`, `verify-android`) |

Targets (canonical prefixed + friendly alias):

| Canonical | Alias | Command | Requires |
|-----------|-------|---------|----------|
| `host-verify` | `verify` | `scripts/verify/host.sh` | — |
| `android-build` | `build-android` | `scripts/verify/android_build.sh` | NDK |
| `android-demo` | `demo` | `scripts/verify/android_demo.sh all` | NDK + device |
| `android-pull` | `pull` | `scripts/verify/android_demo.sh pull` | device |
| `android-list` | `list` | `scripts/verify/android_demo.sh list` | device |
| `android-diff` | `diff` | `scripts/verify/android_demo.sh diff` | device (host ffmpeg) |
| `android-view` | `view` | `scripts/verify/android_demo.sh view` | macOS |
| `android-results` | `results` | `pull` + `diff` + `view` | device |
| `android-demo-live` | `demo-live` | `scripts/verify/android_demo.sh all --live` | NDK + device |
| `android-verify` | `verify-android` | `android-build` + `android-demo` | NDK + device |
| `help` | — | list all targets | — |
| `modules` | — | list registered modules | — |

`ANDROID_NDK_HOME` is read from the environment and forwarded to the scripts.

## 3. File Layout

```text
native_ui/                        # Bazel workspace root
├── Makefile                      # top-level entry (includes mk/*.mk)
├── mk/                           # per-category make modules (AOSP-style registry)
│   ├── rules.mk                  # core registry: register_module / register_target / register_alias
│   ├── help.mk                   # help + modules targets
│   ├── host.mk                   # host verification (host-verify)
│   ├── android.mk                # android build + device flow (android-*)
│   └── aliases.mk                # friendly short aliases
├── scripts/
│   ├── lib/
│   │   ├── common.sh             # shared helpers
│   │   └── pixel_diff.py         # raw RGB frame comparison
│   ├── build_shared.sh           # (kept)
│   ├── tool_fetch_nanosvg.sh     # (renamed from fetch_nanosvg.sh)
│   └── verify/                   # verification flows
│       ├── host.sh
│       ├── android_build.sh
│       └── android_demo.sh
├── out/                          # device artifacts (git-ignored)
```

## 4. Usage

```bash
make help

# Host CI equivalent
make verify

# Android cross-compile (no device)
make build-android

# Device closed loop: build + push + run + decode-back pixel-diff
make demo

# Live-update performance run (prints frame ms + VmRSS)
make demo-live

# Full device validation (T033 + T034)
make verify-android
```

## 5. Validation (no device required)

- `make verify` — host test suite green.
- `make build-android` — android_arm64 cross-build green.
- `make help` — lists targets; `make demo`/`verify-android` stop at the clear
  "no adb device reachable" guard when no device is attached.

## 6. CI Note

`.github/workflows/ci.yml` keeps using raw `bazel build`/`bazel test`. Adopting `make verify`
in CI is optional and can be done later without changing the scripts.

## 7. Open Items / Future Work

- Delete `tool_fetch_nanosvg.sh` once it is no longer useful as a reference.
- Optionally wire `make verify` into `ci.yml`.
- Add `make build` (host `bazel build //...`) and `make format`/`make lint` targets if desired.
