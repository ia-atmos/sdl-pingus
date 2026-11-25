#!/bin/sh
# Plays a level with no screen, and prints the trace of the pingus and
# the result.
#
#   tst/play.sh LEVEL FRAMES [ORDERS] [KEYS]
#
#   LEVEL   a level, as data/levels/tutorial/snow7-grumbel.pingus.atm
#   FRAMES  frames of 25ms (with fast forward, 5 ticks of 20ms each)
#   ORDERS  tick:pingu:action,...  (see game/game.atm)
#   KEYS    more keys, as 300:dn:A (armageddon)
#
# With PINGUS_BIN, it runs that build of main.atm, instead of building.

ROOT=$(cd "$(dirname "$0")/.." && pwd)
ORDERS=$3; [ "$ORDERS" = "-" ] && ORDERS=""
KEYS=$4; [ "$KEYS" = "-" ] && KEYS=""
HOME_DIR=$(mktemp -d)
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy SDL_ATM_MUTE=1 PINGUS_HOME="$HOME_DIR" \
PINGUS_TRACE=1 SDL_ATM_FRAMES="$2:25" PINGUS_ORDERS="$ORDERS" \
SDL_ATM_KEYS="2:dn:Return,3:dn:F${KEYS:+,$KEYS}" \
    ${PINGUS_BIN:-"$ROOT/run.sh" main.atm} "$1"
rm -rf "$HOME_DIR"
