#!/usr/bin/env bash
set -euo pipefail
repo=$(cd "$(dirname "$0")/.." && pwd)
b=${1:?Usage: fetch-sources.sh BUILD_DIRECTORY}
mkdir -p "$b/downloads" "$b/src" "$b/toolchain"
b=$(realpath "$b")
cd "$b/downloads"
fetch() {
  if [[ ! -s "$2" ]]; then
    curl --fail --location --retry 3 --silent --show-error "$1" -o "$2.part"
    mv "$2.part" "$2"
  fi
}
fetch https://github.com/openlgtv/buildroot-nc4/releases/download/webos-b17b4cc/arm-webos-linux-gnueabi_sdk-buildroot.tar.gz webos-sdk.tar.gz
fetch https://codeload.github.com/xbmc/xbmc/tar.gz/refs/tags/21.0-Omega kodi-21.0.tar.gz
fetch https://ffmpeg.org/releases/ffmpeg-6.0.1.tar.xz ffmpeg-6.0.1.tar.xz
fetch https://zlib.net/fossils/zlib-1.3.1.tar.gz zlib-1.3.1.tar.gz
sha256sum --check "$repo/build/downloads.sha256"
if [[ ! -d "$b/toolchain/arm-webos-linux-gnueabi_sdk-buildroot" ]]; then
  tar -xf webos-sdk.tar.gz -C "$b/toolchain"
  (cd "$b/toolchain/arm-webos-linux-gnueabi_sdk-buildroot" && ./relocate-sdk.sh)
fi
if [[ ! -d "$b/src/xbmc-21.0-Omega" ]]; then tar -xf kodi-21.0.tar.gz -C "$b/src"; fi
if [[ ! -d "$b/src/ffmpeg-6.0.1" ]]; then tar -xf ffmpeg-6.0.1.tar.xz -C "$b/src"; fi
if [[ ! -d "$b/src/zlib-1.3.1" ]]; then tar -xf zlib-1.3.1.tar.gz -C "$b/src"; fi
