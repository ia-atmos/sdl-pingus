# sdl-pingus

Pingus in [Atmos](https://github.com/ia-atmos/lang): a port of
[Pingus](https://pingus.seul.org/), a free puzzle game by Ingo Ruhnke
and others, to a language of structured synchronous concurrency.

- The target of the port is the 2018 rewrite of the game logic from
  C++ to Céu ([fsantanna/pingus@036852a][ceu]), the case study
  "Structured Synchronous Reactive Programming for Game Development:
  On Rewriting Pingus from C++ to Céu" (SBGames 2018).
- The port is a third point for that study: C++, Céu, Atmos. See
  [doc/](doc/README.md).
    - The control-flow patterns of the study are the design rule.
    - No state variables for control: no flags, phase enums, or frame
      counters ([doc/state.md](doc/state.md)).
    - The data of the game is Atmos data, read with `X.load`, instead
      of s-expressions with their own reader.

The plan, with the settled decisions and the milestones, is in
[ia-atmos/lang][plan].

[ceu]:  https://github.com/fsantanna/pingus/tree/036852a7718d860baaf2db33cb8c469043df0253
[plan]: https://github.com/ia-atmos/lang/blob/main/.claude/plans/261110-pingus.md

## Run

From nothing, on Linux (Debian, Ubuntu); checked on a clean clone,
with no screen (see below what could not be checked).

1. The packages: a C compiler, `make`, `git`, and SDL2 with its image,
   ttf and mixer libraries:

   ```
   sudo apt-get install build-essential git \
       libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
   ```

2. The three repositories, side by side in one directory (the game
   finds the others as `../lang` and `../env-sdl`):

   ```
   mkdir atmos && cd atmos
   git clone https://github.com/ia-atmos/lang
   git clone https://github.com/ia-atmos/env-sdl
   git clone https://github.com/ia-atmos/sdl-pingus
   git -C lang checkout 9d4561a
   git -C env-sdl checkout 94cb414
   ```

   The compiler (`lang`) and the environment (`env-sdl`: the window, the
   events, images, sounds, the test mode) at the commits with which the
   tests of this repository last passed; `sdl-pingus` at `main`. The two
   go together: a newer compiler may need a newer environment.

3. The compiler (about 15 s):

   ```
   make -C lang
   ```

4. The game, with a window and sound (the first build takes about
   30 s; `./run.sh` builds and runs):

   ```
   cd sdl-pingus
   ./run.sh                                                    # the menu
   ./run.sh main.atm data/levels/tutorial/snow7-grumbel.pingus.atm   # a level
   ./run.sh main.atm data/worldmaps/tutorial.worldmap.atm      # the worldmap
   ./run.sh -o pingus main.atm && ./pingus                     # build once, run many times
   ```

5. Play. The mouse: choose an action in the panel at the left (or with
   the keys `1` to `0`), then click a pingu; the right button dragged
   scrolls the view; a click on the small map moves the view. The keys,
   in a level: `P` pause, `S` one step while paused, `F` or `Space` fast
   forward (while held), `A` armageddon, the arrows scroll, `Escape`
   leaves the level. `F5`, anywhere, shows the options (fullscreen,
   volumes, language, ...). The progress is saved in
   `~/.pingus-atmos.atm` (or in `$PINGUS_HOME`).

6. The tests, with no screen (about 1 minute), and the demos of the
   original game replayed in the port (about 15 minutes):

   ```
   tst/run.sh                  # all tests
   ASAN=1 tst/run.sh           # the same, with the address sanitizer
   tst/demos.sh                # the 159 demos, and how many solve their level
   ```

   The comparison with the official game, tick by tick
   (`tools/compare-all.sh`), also needs the official game built with a
   patch: see `tools/bench/README.md` and `doc/fidelity.md`.

On macOS (not checked): `brew install sdl2 sdl2_image sdl2_ttf
sdl2_mixer`, the same steps 2 and 3, and, before `./run.sh`, the paths
of Homebrew:

```
export ATMOS_CFLAGS="-O2 -I$(brew --prefix)/include"
export ATMOS_LDFLAGS="-L$(brew --prefix)/lib"
```

What was checked in a container with no screen: the packages, the
clones, the build of the compiler and of the game, the menu, a level
and the worldmap run (with screenshots), `tst/run.sh`, `ASAN=1
tst/run.sh`, and `tst/demos.sh`. Not checked: a real window and sound,
and macOS.

## Layout

```
main.atm            the sequence of screens
engine/             the engine
    pingus.h        C of the game: images with modifiers, fonts, ground maps
    sdl.atm         the environment env-sdl, and the Atmos functions over pingus.h
    engine.atm      sprites (a task each), fonts, mouse, widgets
game/               the game screen
    game.atm        layers, clock, pause, armageddon, goal
    hud.atm         buttons, counters, time, small map
    playfield.atm   the view, clicks on pingus
    level.atm       levels, prefabs, groups
    ground.atm      groundpieces
    pingu/          the Pingu task, the mover, particles
        actions/    one file per action (21)
    objs/           one file per object (10 of Céu), extra.atm (12 more)
screens/            menu, start, result, story, credits, levels,
                    worldmap, options
data/               converted data, images, sounds, music
tools/convert.py    s-expressions to Atmos data, with its checks
tools/check.atm     loads every data file with X.load
tools/measure.py    peak memory and CPU of a run
tools/compare.sh    a demo of the original in both games, the traces compared
tools/bench/        the patch of the official game for the comparisons
tst/                tests with no screen
doc/                the case study: C++, Céu, Atmos
```

## Data

The data files of the game are in `data/`, as Atmos data
(`*.pingus.atm`, `*.sprite.atm`, ...). They are generated, once, from
the s-expression files of the 2018 repository (images and sprite
descriptions) and of the official game (levels, levelsets, worldmaps,
prefabs, stories):

```
git clone https://github.com/fsantanna/pingus /tmp/pingus
git -C /tmp/pingus checkout 036852a
git clone https://github.com/Pingus/pingus /tmp/official
git -C /tmp/official checkout ffe3851
tools/merge-data.sh /tmp/pingus/data /tmp/official/data /tmp/src
tools/convert.py /tmp/src data
tools/po.py /tmp/pingus/data/po data/po
```

The header of `tools/convert.py` describes the mapping and what the
conversion loses. `data/levels.atm` is the index of the levels, with
the objects and actions that each one needs: the game offers only the
levels it supports (522 of 523; the last one needs a rocket launcher,
which no version of the game has).

## Test

```
tst/run.sh              # all tests, about 1 minute
tst/run.sh update       # stores the new outputs as the expected ones
ASAN=1 tst/run.sh       # with the address sanitizer and leak checks
tst/play.sh LEVEL FRAMES [ORDERS] [KEYS]    # one level, with its trace
```

All with no screen (`SDL_VIDEODRIVER=dummy`), fixed frames of 25ms, a
fixed random seed, and scripted keys and mouse:

- the data: every converted file loads with `X.load`;
- the engine: a sprite, a text, and a scripted click;
- screenshots of the menu, the levelsets, the start screen, and a game,
  compared with `tst/shots/`;
- the game played with the mouse only (`tst/clicks.sh`);
- 25 scripted levels (`tst/levels.txt`), one per action and per object,
  whose traces of actions are compared with `tst/golden/`.

## Status

- [x] P0 Groundwork: license, converter, data, report of levels
- [x] P1 Engine
- [x] P2 Menus
- [x] P3 Story, credits, worldmap
- [x] P4 Ground
- [x] P5 Pingu core
- [x] P6 Actions and traps
- [x] P7 The rest of the game: pause, fast forward, options, progress
- [x] P8 Case study ([doc/](doc/README.md)), with the benchmark against
      the official game ([doc/bench.md](doc/bench.md))
- [x] P9 Later items: the 12 objects and 4 actions that Céu did not have
- [x] The levels of the official game, and its timings: compared tick by
      tick on its demos: all the 154 that it plays give the same trace
      ([doc/fidelity.md](doc/fidelity.md))
- [x] Translations: the `.po` files of the game, chosen in the options
- [x] The options of the official game (all but its choice of renderer)
- [ ] Input configuration, controllers

## License

Pingus is free software under the GNU General Public License, version
3 or later (see `LICENSE`), and this port is a derivative work under
the same license.

- The authors of the game, of its images, levels, sounds, and music
  are in `AUTHORS` and in `data/credits/`, and in the READMEs of
  `data/`.
- The Céu rewrite is by Francisco Sant'Anna.
