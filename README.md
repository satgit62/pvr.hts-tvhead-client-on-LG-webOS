# Kodi 21 binary add-ons for LG webOS

This repository contains community webOS builds of Inputstream FFmpeg Direct and
the Tvheadend HTSP client. The original ZIP files below target Kodi 21 alpha 3.

## Inputstream FFmpeg Direct: low-memory timeshift build

The source contribution in this branch builds **21.3.8.1** for **Kodi 21 Omega,
32-bit ARM webOS**. It pins the upstream Omega source and applies a small,
reviewable timeshift patch. It does not rebuild or modify `pvr.hts`.

The patch reduces the recent packet window in RAM from 12 minutes to roughly
one minute (plus segment boundaries and the active read segment). Older packets
remain available in the disk buffer. This reduces packet retention on TVs with
limited memory; it is not a hard process-memory cap.

It batches packet fields into one VFS write per packet (retrying short writes),
instead of many small writes to USB, while preserving the segment file format.
It also rebuilds seek indexes for reloaded disk segments, positions a disk seek
at the requested second, resets the packet cursor correctly, guards empty seeks
and pause-before-start, initializes the demux time index, and uses a monotonic
clock for elapsed time.

- [Build and test instructions](docs/BUILD.md)
- [TV installation and USB guidance](docs/INSTALL.md)
- [Source patch](patches/0001-webos-timeshift-memory-and-seek.patch)
- [Validation results and remaining checks](docs/VALIDATION.md)
- [Installable webOS ARM ZIP](inputstream.ffmpegdirect-21.3.8.1-webos-armv7.zip)
- [ZIP SHA-256 checksum](inputstream.ffmpegdirect-21.3.8.1-webos-armv7.zip.sha256)

## Original binary releases

inputstream.ffmpegdirect Add-on for Kodi 21.0-ALPHA3, Compiled for LG webOS.

pvr.hts-tvhead client Add-on for Kodi 21.0-ALPHA3, Compiled for LG webOS.

# Note:

If the installation from the ZIP file fails, unzip the ZIP file and use FileZilla to copy the relevant folder to the /media/developer/apps/usr/palm/applications/org.xbmc.kodi/addons/ folder on the TV.

Have fun with it.

# Quelle: 
https://github.com/xbmc/repo-binary-addons
