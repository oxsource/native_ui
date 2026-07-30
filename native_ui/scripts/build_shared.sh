#!/usr/bin/env bash
set -euo pipefail

# Build the shared library
bazel build //src/framework/public:native_ui_shared

# Copy to dist/
mkdir -p dist
cp bazel-bin/src/framework/public/libnative_ui_shared.* dist/ 2>/dev/null || true

echo "Shared library copied to dist/"
ls -la dist/
