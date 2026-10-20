#!/usr/bin/env bash
set -euo pipefail
b=${1:?Usage: build-dependencies.sh BUILD_DIRECTORY}
b=$(realpath "$b")
sdk=$b/toolchain/arm-webos-linux-gnueabi_sdk-buildroot
prefix=$b/deps
cc=$sdk/bin/arm-webos-linux-gnueabi-gcc
export PATH="$sdk/bin:$PATH"
mkdir -p "$prefix" "$b/build"
if [[ ! -f "$prefix/lib/libz.a" ]]; then
 cd "$b/src/zlib-1.3.1"
 CC="$cc" AR=arm-webos-linux-gnueabi-ar RANLIB=arm-webos-linux-gnueabi-ranlib CFLAGS='-O2 -fPIC -march=armv7-a -mfloat-abi=softfp -mfpu=neon' ./configure --static --prefix="$prefix" > "$b/build/zlib-configure.log"
 make -j8 > "$b/build/zlib-build.log" 2>&1
 make install >> "$b/build/zlib-build.log" 2>&1
fi
if [[ ! -f "$prefix/lib/libpostproc.a" ]]; then
 mkdir -p "$b/build/ffmpeg"
 cd "$b/build/ffmpeg"
 "$b/src/ffmpeg-6.0.1/configure" --prefix="$prefix" --enable-cross-compile --cross-prefix="$sdk/bin/arm-webos-linux-gnueabi-" --arch=arm --target-os=linux --cpu=armv7-a --enable-neon --enable-pic --enable-static --disable-shared --disable-debug --disable-doc --disable-programs --disable-avdevice --enable-gpl --enable-avfilter --enable-postproc --disable-encoders --disable-muxers --disable-autodetect --enable-network --enable-zlib --extra-cflags="-O2 -fPIC -march=armv7-a -mfloat-abi=softfp -mfpu=neon -I$prefix/include" --extra-ldflags="-L$prefix/lib" > "$b/build/ffmpeg-configure.log" 2>&1
 make -j8 > "$b/build/ffmpeg-build.log" 2>&1
 make install >> "$b/build/ffmpeg-build.log" 2>&1
fi
printf 'Cross-compiled FFmpeg and zlib ready\n'
