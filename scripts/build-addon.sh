#!/usr/bin/env bash
set -euo pipefail
repo=$(cd "$(dirname "$0")/.." && pwd)
b=${1:?Usage: build-addon.sh BUILD_DIRECTORY}
b=$(realpath "$b")
sdk=$b/toolchain/arm-webos-linux-gnueabi_sdk-buildroot
kodi=$b/src/xbmc-21.0-Omega
prefix=$b/deps
src=$b/src/inputstream.ffmpegdirect
mkdir -p "$b/kodi-sdk" "$b/build"
bash "$repo/scripts/prepare-source.sh" "$src"
cat > "$b/kodi-sdk/KodiConfig.cmake" <<EOF
set(APP_NAME Kodi)
set(APP_NAME_LC kodi)
set(APP_NAME_UC KODI)
set(APP_PACKAGE org.xbmc.kodi)
set(APP_VERSION_MAJOR 21)
set(APP_VERSION_MINOR 0)
set(APP_VERSION_CODE 21.0)
set(KODI_PREFIX "$b/kodi-sdk")
set(KODI_INCLUDE_DIR "$kodi/xbmc/addons/kodi-dev-kit/include/kodi")
set(KODI_LIB_DIR "$b/kodi-sdk/lib/kodi")
set(KODI_DATA_DIR "$b/kodi-sdk/share/kodi")
set(CORE_SYSTEM_NAME linux)
set(PLATFORM linux)
set(PLATFORM_TAG linux-armv7)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
list(APPEND CMAKE_MODULE_PATH "$kodi/cmake/scripts/common")
add_definitions(-DBUILD_KODI_ADDON -DTARGET_POSIX -DTARGET_LINUX -DTARGET_WEBOS)
include(AddonHelpers)
EOF
cat > "$b/toolchain.cmake" <<EOF
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER "$sdk/bin/arm-webos-linux-gnueabi-gcc")
set(CMAKE_CXX_COMPILER "$sdk/bin/arm-webos-linux-gnueabi-g++")
set(CMAKE_SYSROOT "$sdk/arm-webos-linux-gnueabi/sysroot")
set(CMAKE_FIND_ROOT_PATH "$sdk/arm-webos-linux-gnueabi/sysroot" "$prefix")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
set(CMAKE_C_FLAGS_INIT "-O2 -fPIC -march=armv7-a -mfloat-abi=softfp -mfpu=neon")
set(CMAKE_CXX_FLAGS_INIT "-O2 -fPIC -march=armv7-a -mfloat-abi=softfp -mfpu=neon -static-libstdc++ -static-libgcc")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-Wl,-Bsymbolic")
EOF
export PKG_CONFIG_LIBDIR="$prefix/lib/pkgconfig"
cmake -S "$src" -B "$b/build/addon" -G Ninja \
 -DCMAKE_TOOLCHAIN_FILE="$b/toolchain.cmake" -DKodi_DIR="$b/kodi-sdk" \
 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE='-O2 -DNDEBUG' \
 -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX="$b/stage" -DOVERRIDE_PATHS=ON -DPACKAGE_ZIP=ON \
 -DFFMPEG_PATH="$prefix" -DFFMPEG_INCLUDE_DIRS="$prefix/include" \
 -DFFMPEG_LIBRARIES="$prefix/lib/libavformat.a;$prefix/lib/libavcodec.a;$prefix/lib/libswscale.a;$prefix/lib/libswresample.a;$prefix/lib/libavutil.a;$prefix/lib/libz.a;m;atomic;dl;pthread" \
 -DFFMPEG_LDFLAGS='-Wl,-Bsymbolic' -DZLIB_INCLUDE_DIR="$prefix/include" -DZLIB_LIBRARY="$prefix/lib/libz.a" \
 -DBZIP2_INCLUDE_DIRS="$sdk/arm-webos-linux-gnueabi/sysroot/usr/include" \
 -DBZIP2_LIBRARIES="$sdk/arm-webos-linux-gnueabi/sysroot/usr/lib/libbz2.a" > "$b/build/addon-configure.log" 2>&1
cmake --build "$b/build/addon" -j8 > "$b/build/addon-build.log" 2>&1
cmake --install "$b/build/addon" > "$b/build/addon-install.log" 2>&1
echo 'Native add-on build complete'
