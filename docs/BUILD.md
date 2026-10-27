# Building the Omega webOS timeshift variant

Use Linux x86-64 or Ubuntu 24.04 under WSL2. Build directories must be on the
Linux filesystem, not a Windows mounted drive. Allow several GB of disk space.

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build pkg-config curl git \
  patch autoconf automake libtool nasm yasm python3 unzip
git submodule update --init upstream/inputstream.ffmpegdirect
bash scripts/fetch-sources.sh "$HOME/webos-addon-build"
bash scripts/build-dependencies.sh "$HOME/webos-addon-build"
bash scripts/build-addon.sh "$HOME/webos-addon-build"
python3 scripts/package.py "$HOME/webos-addon-build" dist
```

`fetch-sources.sh` verifies all four downloads against
[`downloads.sha256`](../build/downloads.sha256) before extraction. The SDK is
relocated to the chosen build directory. Do not move the SDK after relocation.
The add-on source is exported from the pinned submodule commit
`3c596496d2360f7b25ff58886a81cb1273266481` and patched in a fresh directory;
the submodule remains untouched. To rebuild changed patches, use a new build
directory or remove only the previously generated `src/inputstream.ffmpegdirect`
directory after inspecting it.

Dependencies: Kodi 21.0-Omega headers and build helpers; openlgtv Buildroot SDK
release `webos-b17b4cc` (GCC 12.2); FFmpeg 6.0.1; zlib 1.3.1; the SDK's bzip2.
FFmpeg is compiled as PIC static libraries with GPL components, network support,
demuxers, parsers and decoders. Encoders, muxers, programs and automatic external
dependency discovery are disabled. Kodi's VFS supplies HTTP(S) input. This is
not a claim of support for every streaming protocol or DRM system.

The native shared object uses ARMv7 NEON and the softfp ABI. C++ runtime libraries
are linked statically. Build logs are in `build/`, installed output is in `stage/`.
The ZIP and SHA-256 sidecar are written to `dist/`. The package includes one
versioned library, `BUILD.json` provenance, and component license notices. The
metadata uses `linux-armv7`, the platform tag recognized by Kodi 21 on webOS.
Use only with Kodi 21's InputStream API 3.3.0, not Kodi 22.

## Regression tests

```sh
sudo apt-get install -y libavcodec-dev libavutil-dev
bash scripts/test.sh "$HOME/webos-timeshift-tests"
```

This compiles the actual patched `TimeshiftBuffer`, `TimeshiftSegment` and
`DiskUtils` source against a small host Kodi VFS/API shim. AddressSanitizer and
UndefinedBehaviorSanitizer are enabled. Five scenarios cover batched packet
writes and short-write retries, disk reload and
repeated seeking, cursor reset/index restoration, unwritable storage, empty
seeks and buffer-file cleanup. It does not emulate the webOS decoder, tuner,
network provider or Kodi player threads.

The `disk-seek` scenario fails against the pinned unpatched source because old
packets remain in RAM, and passes with the patch. The complete patched suite
passes on Ubuntu 24.04. Device validation is recorded separately.

## Licensing and source

The original repository license remains in `LICENSE`. The pinned add-on has
its own `LICENSE.md` (GPL-2.0-or-later) and individual source notices. FFmpeg is
built with `--enable-gpl`; retain its license and corresponding source when
redistributing binaries. Keep the pinned source, patches, dependency versions,
build scripts and checksums available with any published binary.
