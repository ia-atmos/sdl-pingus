# Fidelity: the demos of the original

The data of Pingus has 159 demos (`*.pingus-demo`): recordings of the
actions given to the pingus, by tick of the world and by pingu, most of
them solving their level. `tools/demo.py` turns a demo into the orders
of the test mode of the port, and `tst/demos.sh` replays them all and
counts the levels still solved.

| Build | Solved | Failed | No result |
|-------|--------|--------|-----------|
| first version (release every `rate` ticks, exits before the pingus) | 8 | 82 | 69 |
| release every `rate + 1` ticks, objects behind the pingus react after them (`:tock`), orders of the same tick together, two Escapes to end | 10 | 143 | 6 |
| landing in the same tick as the collision, velocities clamped | 10 | 143 | 6 |
| the fixes of the comparison with the official game (below), 149 of 154 traces as the official game | 12 | 141 | 6 |

Solved now: the bomber, bridger, climber, digger and miner tutorials,
`crystal2`, `crystalcave3-gunter`, `hellmouth04-grumbel`,
`hellmouth08-grumbel`, `hellmouth10-grumbel`, `panic-cbrucher`,
`winter2-chei`.

Few demos solve their level in the official game either: they were
recorded with older engines, and only 38 of the 154 that the official
game plays save any pingu there. The measure of fidelity is therefore
the comparison of traces below, not the count of solved levels.

A demo replays only if the game is the same, tick by tick: one pingu one
pixel off at the time of a click misses the click.

## Tick by tick against the official game

The official game, `Pingus/pingus` at `ffe3851`, builds in the
container (see `tools/bench/README.md`), with a patch
(`tools/bench/official.patch`) that prints the same trace as the port
(`PINGUS_TRACE=1`: a line per change of action, `time pingu id action
x y`), and plays demos at 100 updates per frame. `tools/compare.sh`
replays a demo in both and compares the traces; `tools/compare-all.sh`
does it for all 159 demos. `PINGUS_TRACE=2` with `PINGUS_TRACE_ID`
prints one pingu each tick, in both, to find the first tick where they
differ.

| Build | Same trace | Differ | No official trace |
|-------|-----------|--------|-------------------|
| float mover, before the old level paths were linked | 39 | 33 | 87 |
| all the fixes below | 142 | 12 | 5 |
| the compiler `ia-atmos/lang` `c467e60` (the order of a loop over the pingus while traps kill them) | 145 | 9 | 5 |
| the bomber, the smasher, and the basher as the official update | 149 | 5 | 5 |
| the laser exit, the guillotine, the hammer, and the end of a level as the official update | **154** | 0 | 5 |
| the timings of a tick relaxed (`17480e9`, `49c7fff`): see the deliberate differences | 146 | 8 | 5 |

The faithful version is the commit `3ef9c0f` (2026-11-20): any later
change that alters a trace on purpose is listed here as a deliberate
difference.

So, at `3ef9c0f`, all the 154 demos that the official game plays gave
the same trace in the port: every pingu, every change of action, at the same
tick and the same pixel, up to the tick 39000.

The last five were found the same way, by the per-tick trace of the
first pingu that differs, each a tick of the official update that the
port had not followed:

- `snow1`: the official laser kill checks whether its clock has
  finished before it advances it; the pingu dies at the tick after
  the last frame.
- `xmas-blockblow-timpany`: the official guillotine frees itself at
  the start of the update in which its kill ends, and may kill again in
  that same update.
- `stone-castle-timpany`: the official hammer counts 13 steps and
  splashes when the count reaches the last frame, one tick before the
  end of its animation, each way.
- `pingusworld3-pacho`: the official game ends a level when the time
  is past 125 ticks after the goal, so 126 ticks.
- `indiana-yingwan` was fixed by these.

Four more were fixed by reading the per-tick trace of the first pingu
that differs (`PINGUS_TRACE=2`) in both games:

- `intensiv-timpany`: the bomber died at the end of its animation
  before the move of that tick; the original advances its clock,
  moves, and then dies. It also checks drowning and splashing only up
  to the frame 9 of its clock.
- `real6`: the smasher killed when its animation ended, one tick after
  the original, which kills when its count reaches the last frame; it
  may also trigger again one tick earlier.
- `stone5-grumbel`: the basher looked for something to bash one pixel
  short of the original (`x <= bash_reach`), and its first check, which
  in the original finds something without looking, did look.
- `hellmouth05-grumbel` was fixed by the same changes.

Three more (`aliens4-phil`, `aliens6-phil`, `pacman3-yingwan`) differed
by the order of the pingus that a trap killed in the same tick. A
commit of the compiler, `c467e60`, made a loop over a pool tolerate the
aborts of its tasks in its body, as the loop of a trap over the pingus
does; with it, the order is the one of the official game. Found when
the port moved to the new rules of `pin` (the traces of the port
changed in these three, and only in them).

Most of the rest are the order of the pingus killed in the same tick by the same
object, or a fall that ends one pixel apart, from the details of the
float arithmetic of the faller and of the ground removed by bombers.

The 5 demos the official game does not load (`crystal-10-rz`,
`desert5-tflavel-original`, `egypts1-judit`, `hellmouth-23-rz+eriksoe`,
`hellmouth-24-rz`) name levels or objects it no longer has.

## What the comparison found

Read in the official code first:

- an entrance releases a pingu when the time is *past* the last release
  plus the rate: one every `rate + 1` ticks;
- in a tick, the pingus move first, then the objects react (exits,
  traps), so the objects drawn behind the pingus use a second phase,
  `:tock`;
- a faller decides how it lands in the tick of the collision (the Céu
  rewrite waited one tick, as a workaround), and the velocities are
  clamped to the terminal velocity.

Found with the traces of both games:

- positions and velocities are `float` (32 bits), as the original:
  the build has `ATMOS_FLOAT=32` (`run.sh`), so the reals of Atmos are
  `float`, and the mover and the gravity are in Atmos (the mover was in
  C, `pg_move`, until #18);
- objects are at their real positions (the conversion had rounded
  them), and are set up in the order of their depth, which decides
  which of two objects acts first on a pingu in a tick;
- the orders of the player apply after the objects of the tick;
- exits and traps force an action (`:pingu.set`), while the player
  and the armageddon request one (`:pingu.go`), checked by the change
  rules of the official game (a faller only becomes a floater, a
  climber or a bomber; a bomber or a splashed pingu does not change);
- the timings of the actions follow the clocks of the official game:
  the digger digs every 4th tick, the miner every 4, the basher by its
  frame, the bridger in steps of 14, 16 and 9 ticks, the waiter 82
  ticks, the splashed 16, the bomber dies at the end of its animation;
- the ice melts while a pingu was on it in the last 1000 ticks, the
  spike keeps its own clock, and a teleporter checks every tick.

A danger (water, out of the screen) no longer aborts an action from
outside: as the official game, the actions check the water themselves,
and only leaving the screen kills a pingu at any time.

## Deliberate differences

After the faithful version, the timings of a tick that are accidents of
the official update went back to the forms of Atmos (an action ends
when its animation ends, a trap acts at a frame of its sprite), keeping
the intent: what kills, what saves, and about when. Each differs from
the official game by a tick or a few pixels, and in 8 demos that
changes a trace (2026-11-25):

| Demo | Cause | The difference |
|------|-------|----------------|
| `hellmouth05-grumbel` | the bomber dies at the end of its animation | a falling bomber dies 8 pixels higher |
| `intensiv-timpany` | the same | bombers die 1 pixel higher |
| `real6` | the smasher smashes when its animation ends | a pingu is smashed 1 tick later, 1 pixel ahead |
| `indiana-yingwan` | the hammer smashes when its animation ends | a pingu walks past the hammer, and is smashed at the next stroke (15 ticks later) |
| `stone-castle-timpany` | the same | a pingu that the original lets pass is smashed |
| `snow1` | the laser kill ends with its animation | the pingus die 1 tick earlier |
| `xmas-blockblow-timpany` | the guillotine checks at each tick, after its kill | another pingu of the queue is taken first, and the order of the deaths changes |
| `pingusworld3-pacho` | the end of a level, 125 ticks after the goal | the level ends 1 tick earlier, before the last change of a pingu |

The other changes of `17480e9` (the basher looks for something to bash
at every check, the spike kills when its frame 3 starts) change no
trace. The relaxed actions are those that the earlier list of fixes
names; the rest of the port is still the official update, tick by tick.

## What the demos do not cover

The comparison checks the trace of the logic only (each change of
action of each pingu, with its tick and position), and only in the
levels that have a demo. It does not see the drawing, the sound, the
screens out of a level, nor what no demo exercises. Counted on the 154
official traces (2026-11-25):

- Actions never exercised: `angel`, `boarder`, `smashed`, `superman`
  (none of them was in the Céu rewrite; they are checked against the
  official code only by reading). `laserkill` is in 1 demo.
- Objects never in a compared level: `fake_exit` (11 levels) and
  `starfield_background` (4, drawing only). Few: `conveyorbelt` (2
  demos), `hammer` (3), `smasher` and `iceblock` (4), `laser_exit` (5),
  `guillotine` and `teleporter` (6).

The rest is exercised widely: `walker` and `faller` in more than 146
demos, `bridger`, `blocker`, `waiter`, `basher` in 77 to 109,
`splashed` in 60, `drown` in 33.

