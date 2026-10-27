#!/usr/bin/env python3
"""Package the built add-on, retaining source/license provenance."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import xml.etree.ElementTree as ET
import zipfile

build = Path(sys.argv[1]).resolve()
destination = Path(sys.argv[2]).resolve()
repo = Path(__file__).resolve().parent.parent
addon = build / 'stage/inputstream.ffmpegdirect'
root = ET.parse(addon / 'addon.xml').getroot()
version = root.attrib['version']
extension = root.find("extension[@point='kodi.inputstream']")
library = extension.attrib['library_linux']
binary = addon / library
sdk = build / 'toolchain/arm-webos-linux-gnueabi_sdk-buildroot/bin'
subprocess.run([str(sdk / 'arm-webos-linux-gnueabi-strip'), '--strip-unneeded', str(binary)], check=True)
header = binary.read_bytes()[:20]
if header[:6] != b'\x7fELF\x01\x01' or header[18:20] != b'\x28\x00':
    raise SystemExit('Expected little-endian 32-bit ARM ELF')
destination.mkdir(parents=True, exist_ok=True)
archive = destination / f'inputstream.ffmpegdirect-{version}-webos-armv7.zip'
metadata = {
    'addon': root.attrib['id'], 'version': version, 'platform': 'webos-armv7-softfp',
    'kodi': '21.0-Omega', 'inputstream_api': '3.3.0',
    'upstream_commit': '3c596496d2360f7b25ff58886a81cb1273266481',
    'patch_sha256': hashlib.sha256((repo / 'patches/0001-webos-timeshift-memory-and-seek.patch').read_bytes()).hexdigest(),
    'ffmpeg': '6.0.1', 'zlib': '1.3.1', 'sdk': 'openlgtv webos-b17b4cc',
    'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
    'source_repository': 'https://github.com/satgit62/pvr.hts-tvhead-client-on-LG-webOS',
}
with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED) as z:
    for path in sorted(addon.rglob('*')):
        # Kodi loads the versioned filename in addon.xml; omit duplicate aliases.
        if path.is_file() and not path.is_symlink():
            z.write(path, 'inputstream.ffmpegdirect/' + path.relative_to(addon).as_posix())
    z.writestr('inputstream.ffmpegdirect/BUILD.json', json.dumps(metadata, indent=2) + '\n')
    for source, name in [
        (build / 'src/inputstream.ffmpegdirect/LICENSE.md', 'LICENSE-addon.md'),
        (build / 'src/ffmpeg-6.0.1/COPYING.GPLv2', 'LICENSE-ffmpeg-GPLv2.txt'),
        (build / 'src/zlib-1.3.1/README', 'LICENSE-zlib.txt'),
        (repo / 'licenses/bzip2-LICENSE', 'LICENSE-bzip2.txt'),
        (repo / 'licenses/GCC-COPYING3', 'LICENSE-GCC-GPLv3.txt'),
        (repo / 'licenses/GCC-COPYING.RUNTIME', 'LICENSE-GCC-Runtime-Exception.txt'),
    ]:
        z.write(source, 'inputstream.ffmpegdirect/licenses/' + name)
checksum = hashlib.sha256(archive.read_bytes()).hexdigest()
(destination / (archive.name + '.sha256')).write_text(checksum + '  ' + archive.name + '\n', encoding='utf-8')
print(archive)
print(checksum)
