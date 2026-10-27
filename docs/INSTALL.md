# Installation and USB timeshift notes

This native add-on requires a supported Kodi 21 webOS installation. Installing
a Linux desktop or Raspberry Pi add-on ZIP will not work on ARM webOS.

Before replacing a working version, close Kodi completely and back up its
`addons/inputstream.ffmpegdirect` directory. Preserve the separate user settings
in `.kodi/userdata/addon_data/inputstream.ffmpegdirect/`. Extract the new add-on
directory into Kodi's application `addons/` directory, preserving ownership for
the Kodi app. Restart Kodi and confirm the displayed version. Restore the backup
with Kodi closed if loading or playback fails.

## USB buffer

Choose a dedicated writable directory on an attached USB drive. Kodi on webOS
is jailed: a USB path visible in a root shell may not be visible inside Kodi.
Verify access from Kodi itself. A rooted system can bind-mount that dedicated
folder into the app's jail; the mount must be recreated if the jail or USB mount
is recreated. Never format the drive or remove unrelated files to install this
add-on. Never silently fall back to the TV's internal storage if USB is absent.

Set FFmpeg Direct's timeshift buffer path to the folder Kodi can actually write.
Enable its disk length limit; 0.25 hours gives a nominal 15-minute rolling window.
The upstream limit is time-based and may grow while paused or behind live TV.
It is not a guaranteed byte limit or protection against disk exhaustion. Stop
playback before unplugging USB. Normal buffer destruction removes session
segment/index files; a power failure can leave files behind.

This patch targets packet memory and seeking. Stream outages, codec support,
hardware decoding, audio synchronization and provider connection limits can
still affect playback and need separate diagnosis.
