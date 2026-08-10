#!/usr/bin/env bash
# Shared helpers for native_ui scripts.
#
# Source with:
#   . "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/lib/common.sh"
#
# Provides: log_info/log_ok/log_warn/log_error/die, bazel_build/bazel_test,
# require_ndk, detect_adb/adb_push/adb_pull, and the workspace ROOT.

# Guard against double-sourcing.
if [[ -n "${NATIVE_UI_COMMON_SH:-}" ]]; then
  return
fi
NATIVE_UI_COMMON_SH=1

set -euo pipefail

# --- Paths ------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"  # .../scripts/lib
SCRIPTS_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"                # .../scripts
ROOT="$(cd "${SCRIPTS_DIR}/.." && pwd)"                      # bazel workspace root

# --- Logging (colorized when stdout is a TTY) -------------------------------
if [[ -t 1 ]]; then
  C_RED=$'\033[31m'
  C_GREEN=$'\033[32m'
  C_YELLOW=$'\033[33m'
  C_CYAN=$'\033[36m'
  C_RESET=$'\033[0m'
else
  C_RED=; C_GREEN=; C_YELLOW=; C_CYAN=; C_RESET=
fi

log_info()  { echo "${C_CYAN}[info]${C_RESET}  $*"; }
log_ok()    { echo "${C_GREEN}[ok]${C_RESET}    $*"; }
log_warn()  { echo "${C_YELLOW}[warn]${C_RESET}  $*" >&2; }
log_error() { echo "${C_RED}[error]${C_RESET} $*" >&2; }
die()       { log_error "$*"; exit 1; }

# --- Bazel ------------------------------------------------------------------
# bazel_build [--config NAME] <target...>
bazel_build() {
  local config=()
  if [[ "${1:-}" == "--config" ]]; then
    config=(--config "$2")
    shift 2
  fi
  log_info "bazel build ${config[*]:-} $*"
  (cd "${ROOT}" && bazel build "${config[@]}" "$@")
}

# bazel_test <target...>
bazel_test() {
  log_info "bazel test $*"
  (cd "${ROOT}" && bazel test "$@")
}

# Absolute path to the android_arm64 build output. mac/android share the Bazel output
# tree here (Bazel 6 platform-toolchain restriction), so the `bazel-bin` symlink flips
# with the last config; resolve the android output explicitly to stay robust.
android_bin_dir() {
  (cd "${ROOT}" && bazel info bazel-bin --config=android_arm64 2>/dev/null)
}

# --- Android NDK ------------------------------------------------------------
# require_ndk [NDK_HOME] — echoes a usable NDK path, dies otherwise (NDK >= r25b).
require_ndk() {
  local ndk="${1:-${ANDROID_NDK_HOME:-}}"
  if [[ -z "${ndk}" ]]; then
    die "ANDROID_NDK_HOME is not set. Pass --ndk or export it (NDK >= r25b)."
  fi
  [[ -d "${ndk}" ]] || die "NDK path does not exist: ${ndk}"
  local rev="unknown"
  if [[ -f "${ndk}/source.properties" ]]; then
    rev="$(grep -E '^Pkg\.Revision' "${ndk}/source.properties" | sed 's/.*=[[:space:]]*//')"
  fi
  log_info "ANDROID_NDK_HOME=${ndk} (revision ${rev})"
  echo "${ndk}"
}

# --- ADB --------------------------------------------------------------------
# Sets the global ADB array (respecting $ANDROID_SERIAL / DEVICE_SERIAL); dies
# unless a device is reachable.
detect_adb() {
  ADB=(adb)
  if [[ -n "${DEVICE_SERIAL:-}" ]]; then
    ADB=(adb -s "${DEVICE_SERIAL}")
  fi
  if ! "${ADB[@]}" get-state >/dev/null 2>&1; then
    die "no adb device reachable. Connect a device/emulator, pass --device SERIAL, or export ANDROID_SERIAL."
  fi
  log_info "device: $("${ADB[@]}" get-serialno 2>/dev/null)"
}

adb_push() { log_info "adb push $1 -> $2"; "${ADB[@]}" push "$1" "$2" >/dev/null; }
adb_pull() { log_info "adb pull $1 -> $2"; "${ADB[@]}" pull "$1" "$2" >/dev/null 2>&1 || true; }
