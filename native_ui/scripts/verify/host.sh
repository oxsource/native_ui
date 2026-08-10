#!/usr/bin/env bash
# Host verification: run the full host test suite (feature 011, T032).
. "$(cd "$(dirname "${BASH_SOURCE[0]}")/../lib" && pwd)/common.sh"

bazel_test //tests/... //tests/golden/... //tests/integration/...
log_ok "host verification passed"
