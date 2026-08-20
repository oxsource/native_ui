#!/usr/bin/env bash
# Android font_demo device closed loop (feature 012): build, push the demo binary
# + a font file, run on device, pull the rendered PNG.
#
# Usage:
#   scripts/verify/android_font_demo.sh build [--ndk HOME]
#   scripts/verify/android_font_demo.sh push  [--device SERIAL]
#   scripts/verify/android_font_demo.sh run   [--device SERIAL] [--out DIR]
#   scripts/verify/android_font_demo.sh pull  [--device SERIAL] [--out DIR]
#   scripts/verify/android_font_demo.sh all   [--device SERIAL] [--out DIR] [--ndk HOME]
#
# Subcommands:
#   build   bazel build --config=android_arm64 //examples:font_demo
#   push    adb push binary + Roboto-Regular.ttf to /data/local/tmp
#   run     adb run font_demo (registers <pushed font>), then pull the PNG
#   pull    pull font_demo.png into OUT_DIR
#   all     build + push + run + pull in one go
#
# Defaults:
#   - source font: <root>/tests/assets/fonts/roboto_regular.ttf
#   - out dir:     <root>/out
#   - device:      first adb device, or $ANDROID_SERIAL
. "$(cd "$(dirname "${BASH_SOURCE[0]}")/../lib" && pwd)/common.sh"

BIN_NAME="font_demo"
BIN="$(android_bin_dir)/examples/${BIN_NAME}"
DEVICE_TMP="/data/local/tmp"
REMOTE_BIN="${DEVICE_TMP}/${BIN_NAME}"
REMOTE_FONT="${DEVICE_TMP}/roboto_regular.ttf"
REMOTE_PNG="${DEVICE_TMP}/font_demo.png"

FONT_LOCAL="${ROOT}/tests/assets/fonts/roboto_regular.ttf"
OUT_DIR="${ROOT}/out"
NDK_HOME=""
DEVICE_SERIAL=""

cmd_build() {
  require_ndk "${NDK_HOME}"
  bazel_build --config android_arm64 //examples:font_demo
  [[ -x "${BIN}" ]] || die "build did not produce ${BIN}"
  log_ok "built ${BIN}"
}

cmd_push() {
  [[ -x "${BIN}" ]] || die "${BIN} not found. Run 'build' first."
  detect_adb
  [[ -f "${FONT_LOCAL}" ]] || die "source font not found: ${FONT_LOCAL}"
  adb_push "${BIN}" "${REMOTE_BIN}"
  "${ADB[@]}" shell chmod 755 "${REMOTE_BIN}" >/dev/null
  adb_push "${FONT_LOCAL}" "${REMOTE_FONT}"
}

cmd_run() {
  detect_adb
  log_info "run font_demo on device with font ${REMOTE_FONT}"
  set +e
  "${ADB[@]}" shell "${REMOTE_BIN} ${REMOTE_FONT} ${REMOTE_PNG}" 2>&1
  local rc=$?
  set -e
  [[ ${rc} -eq 0 ]] || log_warn "font_demo exited with code ${rc}"
  cmd_pull
}

cmd_pull() {
  detect_adb
  mkdir -p "${OUT_DIR}"
  adb_pull "${REMOTE_PNG}" "${OUT_DIR}/font_demo.png"
  if [[ -f "${OUT_DIR}/font_demo.png" ]]; then
    log_ok "rendered PNG at ${OUT_DIR}/font_demo.png"
  else
    log_warn "no rendered PNG pulled (demo may have failed on device)"
  fi
  ls -la "${OUT_DIR}" 2>/dev/null || true
}

# --- arg parsing -------------------------------------------------------------
[[ $# -gt 0 ]] || { grep '^#' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 1; }
SUBCMD="$1"; shift

while [[ $# -gt 0 ]]; do
  case "$1" in
    --device) shift; DEVICE_SERIAL="$1" ;;
    --out)    shift; OUT_DIR="$1" ;;
    --ndk)    shift; NDK_HOME="$1" ;;
    -h|--help) grep '^#' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) die "unknown arg: $1" ;;
  esac
  shift
done

case "${SUBCMD}" in
  build) cmd_build ;;
  push)  cmd_push ;;
  run)   cmd_run ;;
  pull)  cmd_pull ;;
  all)   cmd_build; cmd_push; cmd_run ;;
  *) die "unknown subcommand: ${SUBCMD} (use build|push|run|pull|all)" ;;
esac