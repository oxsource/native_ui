#!/usr/bin/env bash
# Android external_image_demo device closed loop + decode-back pixel-diff.
#
# Usage:
#   scripts/verify/android_demo.sh build [--ndk HOME]
#   scripts/verify/android_demo.sh push  [--device SERIAL] [--png LOCAL_PNG]
#   scripts/verify/android_demo.sh run   [--live[=SECONDS]] [--device SERIAL]
#   scripts/verify/android_demo.sh pull  [--device SERIAL] [--out DIR]
#   scripts/verify/android_demo.sh list  [--device SERIAL]
#   scripts/verify/android_demo.sh diff  [--device SERIAL] [--out DIR]
#   scripts/verify/android_demo.sh view  [--out DIR]
#   scripts/verify/android_demo.sh all   [--live[=SECONDS]] [--device SERIAL] [--png LOCAL_PNG] [--out DIR] [--ndk HOME]
#
# Subcommands:
#   build   bazel build --config=android_arm64 //examples:external_image_demo
#   push    adb push binary + source png to /data/local/tmp
#   run     adb run the demo (optionally --live), then pull artifacts into OUT_DIR
#   pull    pull the last run's artifacts (MP4 + PNGs) into OUT_DIR
#   list    ls the demo artifacts on the device
#   diff    decode-back pixel-diff: MP4 frame 0 vs the demo's CPU snapshot (T033)
#   view    open the pulled artifacts (MP4 + PNGs) for visual comparison
#   all     build + push + run + pull + diff in one go
#
# Defaults:
#   - source png: <root>/assets/photo/police.png
#   - out dir:    <root>/out
#   - device:     first adb device, or $ANDROID_SERIAL
. "$(cd "$(dirname "${BASH_SOURCE[0]}")/../lib" && pwd)/common.sh"

BIN_NAME="external_image_demo"
BIN="${ROOT}/bazel-bin/examples/${BIN_NAME}"
DEVICE_TMP="/data/local/tmp"
REMOTE_BIN="${DEVICE_TMP}/${BIN_NAME}"
REMOTE_PNG="${DEVICE_TMP}/police.png"
REMOTE_MP4="${DEVICE_TMP}/external_image.mp4"
REMOTE_CPU_PNG="${DEVICE_TMP}/external_image_cpu.png"
REMOTE_SRC_PNG="${DEVICE_TMP}/source_buffer.png"

PNG_LOCAL="${ROOT}/assets/photo/police.png"
OUT_DIR="${ROOT}/out"
NDK_HOME=""
DEVICE_SERIAL=""
LIVE_ARG=""

cmd_build() {
  require_ndk "${NDK_HOME}"
  bazel_build --config android_arm64 //examples:external_image_demo
  [[ -x "${BIN}" ]] || die "build did not produce ${BIN}"
  log_ok "built ${BIN}"
}

cmd_push() {
  [[ -x "${BIN}" ]] || die "${BIN} not found. Run 'build' first."
  detect_adb
  [[ -f "${PNG_LOCAL}" ]] || die "source PNG not found: ${PNG_LOCAL} (use --png)"
  adb_push "${BIN}" "${REMOTE_BIN}"
  "${ADB[@]}" shell chmod 755 "${REMOTE_BIN}" >/dev/null
  adb_push "${PNG_LOCAL}" "${REMOTE_PNG}"
}

cmd_run() {
  detect_adb
  log_info "run on device${LIVE_ARG:+ (${LIVE_ARG})}"
  set +e
  "${ADB[@]}" shell "${REMOTE_BIN} ${LIVE_ARG:+${LIVE_ARG}}" 2>&1
  local rc=$?
  set -e
  [[ ${rc} -eq 0 ]] || log_warn "demo exited with code ${rc}"
  cmd_pull
}

cmd_pull() {
  detect_adb
  mkdir -p "${OUT_DIR}"
  adb_pull "${REMOTE_MP4}"     "${OUT_DIR}/external_image.mp4"
  adb_pull "${REMOTE_CPU_PNG}" "${OUT_DIR}/external_image_cpu.png"
  adb_pull "${REMOTE_SRC_PNG}" "${OUT_DIR}/source_buffer.png"
  log_ok "artifacts in ${OUT_DIR}:"
  ls -la "${OUT_DIR}"
}

cmd_list() {
  detect_adb
  log_info "device artifacts in ${DEVICE_TMP}:"
  "${ADB[@]}" shell "ls -l ${REMOTE_BIN} ${REMOTE_MP4} ${REMOTE_CPU_PNG} ${REMOTE_SRC_PNG}" 2>&1 || \
    log_warn "some artifacts not present on device yet"
}

cmd_view() {
  command -v open >/dev/null 2>&1 || die "open (macOS) is required to view artifacts"
  local files=( "${OUT_DIR}/external_image.mp4" "${OUT_DIR}/external_image_cpu.png"
                "${OUT_DIR}/source_buffer.png" )
  for f in "${files[@]}"; do
    if [[ -f "${f}" ]]; then
      log_info "open ${f}"
      open "${f}"
    else
      log_warn "missing ${f} — run 'pull' first"
    fi
  done
}

cmd_diff() {
  local mp4="${OUT_DIR}/external_image.mp4"
  local cpu="${OUT_DIR}/external_image_cpu.png"
  [[ -f "${mp4}" ]] || die "missing ${mp4} — run 'run' first"
  [[ -f "${cpu}" ]] || die "missing ${cpu} — run 'run' first"
  command -v ffmpeg >/dev/null 2>&1 || die "ffmpeg is required for the decode-back diff"
  command -v python3 >/dev/null 2>&1 || die "python3 is required for the decode-back diff"

  # Dimensions from the CPU snapshot. ffmpeg -i exits 1 when only probing, so the
  # pipeline must not trip set -e (pipefail).
  local dims w h
  dims="$(ffmpeg -i "${cpu}" -hide_banner 2>&1 | grep -oE '[0-9]{2,5}x[0-9]{2,5}' | head -1)" || true
  w="${dims%x*}"
  h="${dims#*x}"
  [[ -n "${w}" && -n "${h}" ]] || die "could not determine dimensions of ${cpu}"

  log_info "decode-back: frame 0 of ${mp4} vs ${cpu} (${w}x${h})"
  # The hardware encoder may pad the coded width (e.g. 208 -> 256); the rendered
  # content is at the top-left, so crop frame 0 to the CPU snapshot dimensions.
  ffmpeg -y -i "${mp4}" -frames:v 1 -vf "crop=${w}:${h}:0:0" -f rawvideo -pix_fmt rgb24 \
      "${OUT_DIR}/frame0.rgb" >/dev/null 2>&1 || die "ffmpeg: could not extract MP4 frame 0"
  ffmpeg -y -i "${cpu}" -f rawvideo -pix_fmt rgb24 "${OUT_DIR}/cpu.rgb" \
      >/dev/null 2>&1 || die "ffmpeg: could not decode the CPU snapshot"

  if python3 "${SCRIPTS_DIR}/lib/pixel_diff.py" \
      "${OUT_DIR}/frame0.rgb" "${OUT_DIR}/cpu.rgb" "${w}" "${h}"; then
    log_ok "decode-back pixel-diff PASSED (MP4 frame 0 matches the CPU snapshot)"
  else
    log_error "decode-back pixel-diff FAILED (MP4 frame 0 differs from the CPU snapshot)"
    exit 1
  fi
}

# --- arg parsing -------------------------------------------------------------
[[ $# -gt 0 ]] || { grep '^#' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 1; }
SUBCMD="$1"; shift

while [[ $# -gt 0 ]]; do
  case "$1" in
    --device) shift; DEVICE_SERIAL="$1" ;;
    --png)    shift; PNG_LOCAL="$1" ;;
    --out)    shift; OUT_DIR="$1" ;;
    --ndk)    shift; NDK_HOME="$1" ;;
    --live*)  LIVE_ARG="$1" ;;
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
  list)  cmd_list ;;
  diff)  cmd_diff ;;
  view)  cmd_view ;;
  all)   cmd_build; cmd_push; cmd_run; cmd_diff ;;
  *) die "unknown subcommand: ${SUBCMD} (use build|push|run|pull|list|diff|view|all)" ;;
esac
