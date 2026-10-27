#!/usr/bin/env bash
set -euo pipefail
repo=$(cd "$(dirname "$0")/.." && pwd)
build=${1:?Usage: test.sh NEW_BUILD_DIRECTORY}
bash "$repo/scripts/prepare-source.sh" "$build/source"
cmake -S "$repo/tests" -B "$build/tests" -G Ninja -DADDON_SOURCE="$(realpath "$build/source")"
cmake --build "$build/tests" --parallel 4
ctest --test-dir "$build/tests" --output-on-failure
