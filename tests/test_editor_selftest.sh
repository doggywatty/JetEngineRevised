#!/bin/bash
# JetEditor M3, headless path, end to end. A project is created with `jeteditor --new`, two files are imported with
# `jeteditor --import` (an uncompressed TGA and a shader XML) and one is refused (a .wav, which must be logged as an
# unsupported type). `jeteditor --selftest` must exit 0 on that project. Copies that no longer match the database (a
# changed file, a missing file, an unlisted file, an empty project) must make it exit non-zero.
# With a MinionRush data folder (third argument, or JET_DATA_DIR), `jeteditor --world-edit` must load levels/level.bclara
# in edit mode (no gameplay state) and print its entities. Without one that check is skipped and says so.
#   tests/test_editor_selftest.sh <jeteditor binary> [work dir] [data dir]
set -u
BIN=$(realpath "$1")
WORK=${2:-$(mktemp -d)}
DATA=${3:-${JET_DATA_DIR:-}}
rm -rf "$WORK"
mkdir -p "$WORK/src"
fail=0
check() {
    if [ "$2" -eq 0 ]; then echo "ok   $1"; else echo "FAIL $1"; fail=1; fi
}

# Fixtures: a 2x2 32-bit uncompressed TGA, a minimal shader technique, and a file with a .wav name
python3 - "$WORK/src" <<'EOF'
import struct, sys, os
d = sys.argv[1]
header = struct.pack('<BBBHHBHHHHBB', 0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 32, 0x08)
pixels = bytes([0, 0, 255, 255, 0, 255, 0, 255, 255, 0, 0, 255, 255, 255, 255, 255])
open(os.path.join(d, 'atlas.tga'), 'wb').write(header + pixels)
open(os.path.join(d, 'blend.xml'), 'w').write('<technique name="blend"/>\n')
open(os.path.join(d, 'sfx.wav'), 'wb').write(b'RIFF' + bytes(40))
EOF

P="$WORK/proj"
"$BIN" --new "$P" "Selftest project" > "$WORK/new.log" 2>&1
check "new project is created" $?
"$BIN" --import "$P" "$WORK/src/atlas.tga" > "$WORK/import1.log" 2>&1
check "import accepts the TGA" $?
"$BIN" --import "$P" "$WORK/src/blend.xml" > "$WORK/import2.log" 2>&1
check "import accepts the shader XML" $?
"$BIN" --import "$P" "$WORK/src/sfx.wav" > "$WORK/import3.log" 2>&1
rc=$?
[ $rc -eq 1 ] && grep -q "^error: Import asset failed: unsupported asset type '.wav'" "$WORK/import3.log"
check "import refuses the .wav and logs it as unsupported" $?

"$BIN" --selftest "$P" > "$WORK/selftest.log" 2>&1
check "self-test passes on the imported project" $?
grep -q "^PASS: all " "$WORK/selftest.log"
check "self-test prints PASS" $?
grep -q "^selected textures/atlas.tga  type=tga\|^selected textures/atlas.tga  type=pvr3\|^selected shaders/blend.xml  type=shader" "$WORK/selftest.log"
check "self-test selects the first asset in tree order" $?

# A changed file: the database still records the size and hash of the imported copy
cp -r "$P" "$WORK/changed"
printf 'x' >> "$WORK/changed/textures/atlas.tga"
"$BIN" --selftest "$WORK/changed" > "$WORK/changed.log" 2>&1
rc=$?
[ $rc -ne 0 ] && grep -q "^FAIL: " "$WORK/changed.log"
check "self-test fails on a file that differs from the database" $?

# A missing file
cp -r "$P" "$WORK/missing"
rm "$WORK/missing/shaders/blend.xml"
"$BIN" --selftest "$WORK/missing" > "$WORK/missing.log" 2>&1
rc=$?
[ $rc -ne 0 ]
check "self-test fails on a database file that is gone" $?

# A file in a layout folder that the database does not list
cp -r "$P" "$WORK/extra"
printf 'x' > "$WORK/extra/models/unlisted.pig"
"$BIN" --selftest "$WORK/extra" > "$WORK/extra.log" 2>&1
rc=$?
[ $rc -ne 0 ] && grep -q "extra:   models/unlisted.pig" "$WORK/extra.log"
check "self-test fails on a file the database does not list" $?

# A project with no assets has nothing to select
"$BIN" --new "$WORK/empty" "Empty project" > /dev/null 2>&1
"$BIN" --selftest "$WORK/empty" > "$WORK/empty.log" 2>&1
rc=$?
[ $rc -ne 0 ]
check "self-test fails on a project with no asset to select" $?

# A folder that does not exist
"$BIN" --selftest "$WORK/does-not-exist" > "$WORK/nofolder.log" 2>&1
rc=$?
[ $rc -ne 0 ]
check "self-test fails on a folder that is not a project" $?

# The level in edit mode: the game loads level.bclara, the menu state it pushes is dropped, and no gameplay state runs
if [ -n "$DATA" ]; then
    "$BIN" --world-edit "$DATA" > "$WORK/worldedit.log" 2>&1
    check "world-edit loads the level with no gameplay state" $?
    grep -q "^world-edit: [1-9][0-9]* entities in levels/level.bclara" "$WORK/worldedit.log"
    check "world-edit prints the entity count" $?
else
    echo "skip world-edit: no data folder (third argument or JET_DATA_DIR)"
fi

if [ $fail -eq 0 ]; then echo "all editor self-test checks ok"; else echo "editor self-test: failures"; fi
exit $fail
