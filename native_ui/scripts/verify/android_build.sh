#!/usr/bin/env bash
# Android arm64 cross-compile verification: framework libs + external_image_demo.
# No device required. Usage: scripts/verify/android_build.sh [--ndk HOME]
. "$(cd "$(dirname "${BASH_SOURCE[0]}")/../lib" && pwd)/common.sh"

NDK_HOME=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    --ndk) shift; NDK_HOME="$1" ;;
    -h|--help) grep '^#' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) die "unknown arg: $1 (use --ndk HOME)" ;;
  esac
  shift
done

require_ndk "${NDK_HOME}"

bazel_build --config android_arm64 \
  //src/framework/surface:surface \
  //src/framework/render:render \
  //src/framework/widgets:widgets \
  //examples:external_image_demo

BIN="${ROOT}/bazel-bin/examples/external_image_demo"
[[ -f "${BIN}" ]] || die "build did not produce ${BIN}"
if ! file "${BIN}" | grep -q 'ARM aarch64'; then
  die "${BIN} is not an aarch64 binary"
fi
log_ok "android_arm64 build verified: ${BIN}"
