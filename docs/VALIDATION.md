# Validation: 21.3.8.1 webOS ARM

Date: 2026-10-20.

## Host and build checks

- Ubuntu 24.04 / WSL2: five CTest scenarios pass with AddressSanitizer and
  UndefinedBehaviorSanitizer. The unpatched Omega source fails the disk-retention
  regression; the patched source passes.
- Packet-write test: ten ordinary packets produce ten VFS writes, rather than
  separate writes for every field. Reloaded payloads and timestamps match.
  A VFS shim limited to seven bytes per write also round-trips a packet.
- Pinned sources and SDK downloads pass SHA-256 verification. The contributed
  build script cross-compiles the native add-on and the packaging script produces
  a ZIP with source/patch provenance and license notices.
- ELF: 32-bit little-endian ARM EABI5, softfp target; required GLIBC symbol
  versions are no newer than 2.7. Dynamic dependencies are the target system's
  libc, libm, libpthread, librt, libatomic and loader. No desktop FFmpeg or dynamic
  C++ runtime dependency is required.

## Device checks

LG UN7000PUC, webOS 5.6.2 / firmware 04.64.00, Kodi 21.0:

- Kodi recognizes version 21.3.8.1; the new binary is mapped into the Kodi process.
- The installed binary SHA-256 matches the packaged binary:
  `00c2411556409651bc95f025b07302630564c96f40791e067c45aaf403031dd2`.
- Live transport-stream playback opens with HEVC 960x540 video and stereo E-AC-3
  audio streams. USB segment/index files are created in the dedicated buffer.
- Pause changes playback speed to zero; resume restores speed one. Kodi accepts
  a rewind request. Buffer files are removed when playback stops.

Sustained smooth playback is **not verified**. The live session showed stalls
while the network was also in use for a video meeting. A separate 20-second
source download received data steadily, so neither network congestion nor an
input/player issue has been isolated. Disk-backed seek correctness is verified
by host regression tests; a long device rewind, lip-sync assessment and a full
TV reboot test remain to be performed under controlled conditions. These results
do not establish that the patch fixes every stall or audio-sync problem.
