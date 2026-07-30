#!/usr/bin/env bash
# nanosvg is now fetched via Bazel http_archive in native_ui_deps.bzl.
# This script is kept as a reference for manual download if needed.
set -euo pipefail

URL="https://github.com/oxsource/nanovg/archive/refs/tags/v1.0.0.tar.gz"
echo "nanosvg is integrated via http_archive in native_ui_deps.bzl"
echo "Run: bazel fetch //third_party/nanosvg"
echo "Or just build any target that depends on it: Bazel fetches automatically."
echo ""
echo "Tarball URL: $URL"
