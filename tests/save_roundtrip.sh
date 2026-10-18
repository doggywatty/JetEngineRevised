#!/bin/bash
# The save game, end to end, through the headless game (test_intro): what the player earns is saved, the process is
# restarted, and the profile is read back; a save written by another device, a damaged save and a damaged settings file
# are refused as Game::ValidateSaveData / SaveVerifierMgr do.
#   tests/save_roundtrip.sh <test_intro binary> <data dir> [work dir]
set -u
BIN=$(realpath "$1")
DATA=$(realpath "$2")
WORK=${3:-$(mktemp -d)}
mkdir -p "$WORK"
fail=0

# run <savedir> <log> [VAR=value ...]: one process in the data dir
run() {
    local sd=$1 log=$2
    shift 2
    (cd "$DATA" && env JET_SAVEDIR="$sd" "$@" timeout 300 "$BIN" "$DATA" "$WORK/frame" 480 854 60000 >"$log" 2>&1)
}
profile() { grep -o "^PROFILE .*" "$1"; }
check() {  # check <description> <condition status>
    if [ "$2" -eq 0 ]; then echo "ok   $1"; else echo "FAIL $1"; fail=1; fi
}

# 1. earn, buy, save, restart
SD=$WORK/sv1; rm -rf "$SD"
run "$SD" "$WORK/1a.log" JET_TUTORIAL=1 JET_GRANT="bananas:12345:3000,tokens:77:3000" JET_BUY="costume:1@4000,bonus:0@4500,settings:30@5000,coppa:64@5100" \
    JET_STAT="bananas:999:5200" JET_MULT="3@6000" JET_SAVE=8000 JET_PROFILE=9000 JET_SHUTDOWN=10000
run "$SD" "$WORK/1b.log" JET_PROFILE=6000 JET_QUIT=7000
A=$(profile "$WORK/1a.log"); B=$(profile "$WORK/1b.log")
[ -n "$A" ] && [ "$A" = "$B" ]; check "profile restored after a restart" $?
echo "$B" | grep -q "bananas=12345 tokens=77" ; check "bananas and tokens" $?
echo "$B" | grep -q "equipped=08_MinionCostume_Firefighter" ; check "costume bought and worn" $?
echo "$B" | grep -q "multiplier=3/0" ; check "missions multiplier" $?
echo "$B" | grep -q "tutorialFirstPlay=1" ; check "tutorial state" $?
echo "$B" | grep -q "musicVolume=0.30 coppa=40" ; check "settings file (volume, age gate)" $?

# 2. a purchase asks for a save that the next update writes, and survives a kill
SD=$WORK/sv2; rm -rf "$SD"
run "$SD" "$WORK/2a.log" JET_BUY="costume:2@4000" JET_QUIT=6000
grep -q "QUIT (save pending: 0)" "$WORK/2a.log"; check "deferred save written by the next update" $?
run "$SD" "$WORK/2b.log" JET_PROFILE=5000 JET_QUIT=6000
profile "$WORK/2b.log" | grep -q "equipped=19_MinionCostume_Dad"; check "deferred save restored" $?

# 3. a save of another device (another GLUID) is refused: the profile starts again
SD=$WORK/sv3; rm -rf "$SD"; cp -r "$WORK/sv1" "$SD"
python3 -c "open('$SD/gluid_enc','wb').write(bytes(range(16)))"
run "$SD" "$WORK/3.log" JET_PROFILE=6000 JET_QUIT=7000
profile "$WORK/3.log" | grep -q "bananas=0 tokens=0"; check "save of another device refused" $?

# 4. damaged saves: a few bytes are repaired from the copies, a destroyed file is kept as savegame_cr and removed
SD=$WORK/sv4; rm -rf "$SD"; cp -r "$WORK/sv1" "$SD"
python3 - "$SD/savegame" <<'EOF'
import sys
d = bytearray(open(sys.argv[1], 'rb').read())
for i in (200, 5000, 9001, 17000, 25000):
    d[i] ^= 0xFF
open(sys.argv[1], 'wb').write(d)
EOF
run "$SD" "$WORK/4a.log" JET_PROFILE=6000 JET_QUIT=7000
profile "$WORK/4a.log" | grep -q "bananas=12345 tokens=77"; check "a few damaged bytes repaired" $?
SD=$WORK/sv5; rm -rf "$SD"; cp -r "$WORK/sv1" "$SD"
python3 - "$SD/savegame" <<'EOF'
import sys
d = bytearray(open(sys.argv[1], 'rb').read())
for i in range(100, len(d) - 100, 3):
    d[i] ^= 0x5A
open(sys.argv[1], 'wb').write(d)
EOF
run "$SD" "$WORK/4b.log" JET_PROFILE=6000 JET_QUIT=7000
grep -q "Invalid save game files. Deleting." "$WORK/4b.log" && [ -f "$SD/savegame_cr" ]; check "destroyed save kept as savegame_cr" $?
profile "$WORK/4b.log" | grep -q "bananas=0 tokens=0"; check "destroyed save: new profile" $?

# 5. a settings file that does not load
SD=$WORK/sv6; rm -rf "$SD"; cp -r "$WORK/sv1" "$SD"
head -c 35 /dev/urandom >"$SD/settings"
run "$SD" "$WORK/5.log" JET_PROFILE=6000 JET_QUIT=7000
grep -q "Invalid settings save files. Deleting." "$WORK/5.log" && [ -f "$SD/settings_cr" ]; check "damaged settings kept as settings_cr" $?
profile "$WORK/5.log" | grep -q "bananas=12345 tokens=77"; check "damaged settings: the profile is kept" $?

exit $fail
