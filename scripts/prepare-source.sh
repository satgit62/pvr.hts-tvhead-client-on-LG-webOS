#!/usr/bin/env bash
set -euo pipefail
repo=$(cd "$(dirname "$0")/.." && pwd)
destination=${1:?Usage: prepare-source.sh NEW_DIRECTORY}
commit=3c596496d2360f7b25ff58886a81cb1273266481
if [[ -e "$destination" ]]; then
  echo 'Destination already exists; use a new directory.' >&2
  exit 1
fi
git -C "$repo/upstream/inputstream.ffmpegdirect" cat-file -e "$commit^{commit}"
mkdir -p "$destination"
git -C "$repo/upstream/inputstream.ffmpegdirect" archive "$commit" | tar -x -C "$destination"
patch --batch --forward -d "$destination" -p1 < "$repo/patches/0001-webos-timeshift-memory-and-seek.patch"
