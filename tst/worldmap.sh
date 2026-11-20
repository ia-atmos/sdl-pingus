#!/bin/sh
# The worldmap with the first level solved: the second dot is open, a
# click on it walks the pingu there, and the position is saved.
ROOT=$(cd "$(dirname "$0")/.." && pwd)
HD=$(mktemp -d)
echo '[levels=[@"data/levels/tutorial/digger-tutorial2-grumbel.pingus.atm"=[solved=true, saved=6]], stories=[@"stories/tutorial_intro.story"=true]]' > "$HD/.pingus-atmos.atm"
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy SDL_ATM_MUTE=1 PINGUS_HOME="$HD" \
SDL_ATM_FRAMES="200:25" SDL_ATM_MOUSE="20:dn:505:290:1,21:up:505:290:1" \
    ${PINGUS_BIN:-"$ROOT/run.sh" main.atm} data/worldmaps/tutorial.worldmap.atm
cat "$HD/.pingus-atmos.atm"
rm -rf "$HD"

# fast forward (F held): the pingu walks 4 times as fast, and arrives in
# 60 frames, which is not enough without it
HD=$(mktemp -d)
echo '[levels=[@"data/levels/tutorial/digger-tutorial2-grumbel.pingus.atm"=[solved=true, saved=6]], stories=[@"stories/tutorial_intro.story"=true]]' > "$HD/.pingus-atmos.atm"
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy SDL_ATM_MUTE=1 PINGUS_HOME="$HD" \
SDL_ATM_FRAMES="60:25" SDL_ATM_MOUSE="20:dn:505:290:1,21:up:505:290:1" SDL_ATM_KEYS="21:dn:F" \
    ${PINGUS_BIN:-"$ROOT/run.sh" main.atm} data/worldmaps/tutorial.worldmap.atm
grep -o 'worldmaps=\[[^]]*\]' "$HD/.pingus-atmos.atm" || echo "not arrived"
rm -rf "$HD"
