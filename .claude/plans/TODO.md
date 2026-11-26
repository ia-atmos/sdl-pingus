# TODO: the port of Pingus

Everything pending in the session of the port: decisions waiting for
the author, work known and not done, reports not acknowledged, older
items. Kept current: an item leaves this file when it is done, with
the commit that did it.

The port itself is complete: the plan (P0..P9), the official data, 21
actions, the full options, the comparison with the official game (154
of 154 demos with the same trace), the benchmark, the new rules of `pin`.

## Index of items (`#N`)

The standing rule of the author (2026-11-18): every item put to him has
a number, unique in this session (`sdl-pingus#N` from other sessions),
never reused. The items written before the rule keep their old
references in the history; the open ones are numbered here.

| # | Item | State |
|---|------|-------|
| #1 | The 5 demos whose traces differ from the official game (A.2) | done, by the session: each cause found and fixed; 154 of 154 identical |
| #2 | Expressiveness over exactness, after full fidelity (A.4) | evaluated 2026-11-25 (options #2a, #2b, #2c put to the author; recommended #2b). The condition holds now (154 of 154); the faithful commit is `3ef9c0f`, recorded in `doc/fidelity.md` (a tag `faithful-154` was refused by the permissions of the session: HTTP 403 on push); an option, ask before a large change |
| #3 | The remote of the checkout has the old address (B.0) | won't do (the author, 2026-11-22): GitHub redirects the pushes |
| #4 | Input configuration and controllers (B.1) | open |
| #5 | Drawing offsets of the bridger and the climber (B.2) | done: the bridger as `get_look` of the original; the climber has no offset in the original |
| #6 | The worldmap: fast forward and its border (B.3) | done: fast forward 4 times, tested; the border, for a map smaller than the screen |
| #7 | A test of blocker, faller, blocker (B.4) | done: `blockfall` in `tst/levels.txt` |
| #8 | Fullscreen and resolution not seen on a real screen (B.5) | waits for #13 |
| #9 | The languages in the order of their codes, not their names (B.6) | done: sorted by name, as the original |
| #10 | Stale counts in `doc/fidelity.md` and `doc/bench.md` (B.7, B.8) | done: fidelity updated (12 solved); bench was right (4 ticks per 20 ms is 5 per frame of 25 ms) |
| #11 | The 5 demos the official game cannot load (B.9) | no action possible |
| #12 | Language features that would help (B.10) | sent to `atmos-lang`, informational |
| #13 | The author's own test of Pingus, with the step by step of the README | done (the author, 2026-11-25): it found #19, #20, #21, all fixed |
| #14 | A pin given away by `until`, missed by the warning (`atmos-pin#11`); fixed in Pingus (`0c6ab51`), and in the compiler on the branch `pin` | done here; waits for the merge |
| #15 | The pilot `atmos-lang#76`: the bomber level as a `test` block (`tst/bomber-test.atm`, `f0c33d5`, `5519184`); the dummy drivers remain, for env-sdl | done, reported |
| #16 | Five more levels as `test` blocks (`tst/levels-test.atm`): digger, blocker, miner, basher, the wall jump; traces as the goldens up to tick 1500 | done, by the session |
| #17 | For the paper (`paper-games#4`): the CPU of the game logic alone, port and original, with the drawing skipped in both (median of 3, ms per frame, the ratio), in `doc/bench.md` | open, by the session |
| #18 | Whether the floats kept in C (`pg_move`, `pg.f32`) can go back to Atmos with `ATMOS_FLOAT=32` | done: `run.sh` builds with `ATMOS_FLOAT=32`, the mover is in Atmos, `pg.f32` is gone (`0c285c7`, `bf7f4cf`); the 159 demo traces unchanged, the ice block now melts as the float of the original; confirmed by the author (2026-11-25) |
| #19 | The sound on entering the worldmap (`letsgo`, `PingusMenu::do_start`) was missing | done: `screens/menu.atm`, on "Story"; confirmed by the author (2026-11-25) |
| #20 | The leave button of the worldmap: not at its corner, no "Leave?" | done: at `(0, H-37)` with "Leave?", as `WorldmapScreenCloseButton`; confirmed by the author (2026-11-25) |
| #21 | On the worldmap, the first mountain and the dots moved a pixel apart while the pingu walked (called #20 by the author in the session) | done: the camera follows the pingu in whole pixels, as the original; confirmed by the author (2026-11-25) |
| #22 | `main.atm`: `val file = ...` and `@+`, the forms of Atmos (numbered #21 in `d5901d2`, a number already taken) | done |
| #23 | Dead code found in the evaluation of #2: the task `Mover` has no caller, and the branch of `Pingu` for a forced `:drown`/`:splashed` cannot run since `Danger` only gives `:dead` | open (removal with identical traces) |
| #24 | The weather costs about 18 ms of CPU per tick in `pingusworld3-pacho` (5 snow generators and rain): a task per flake and per drop, where the original has a vector; found by #17 | open (one task for all the particles of a generator, as the stars; no trace changes) |
| #25 | The laser exit may have the off-by-one that the guillotine had (it waits for the next tick after its animation); no demo exercises it | open (to check against the official code) |
| #26 | (b) The coverage of the demos: never compared, the actions `angel`, `boarder`, `smashed`, `superman` and the object `fake_exit`; few, the laser exit, the conveyor belt, the hammer, the smasher, the ice (`doc/fidelity.md`) | done; the uncovered ones to check by reading against the official code |
| #27 | (a) Screenshots of the official game compared with the port, pixel by pixel, in the screens and levels | open, approved by the author |
| #28 | (a) The start screen of a level: the description wraps one word later, the three lines of numbers are 3 px lower, the author 5 px lower, Play 10 px to the right, than the official | open |
| #29 | (a) The worldmap: the port draws the title of the worldmap, the official does not; and a dot near a rock that the official does not show | open |
| #30 | The bitmap fonts of `engine/pingus.h` in Atmos (proposed, not approved) | proposed |
| #31 | Excerpt, idiomatic (#18): `game/pingu/pingu.atm:173-179`, the `Mover` task, `loop on :tick` and `emit @(:parent) :collided` | shown to the author |
| #32 | Excerpt, bad (#18): `game/pingu/pingu.atm:121-130` and `151-156`, a search in a loop: `until c, true } \|\| false`, as a loop gives no value when it ends with no `until` | shown to the author |
| #33 | The answer to the general instruction for the ports (2026-11-26, via `atmos`): how much the port follows it, what to redo, a grade (7) | answered, waiting for the author |
| #34 | Excerpt, idiomatic (searches as `iter`): `game/pingu/pingu.atm:121-124`, the collision over the height as `iter.any((0, PINGU.height), ...)` | shown to the author |
| #35 | Excerpt, bad (searches as `iter`): `screens/worldmap.atm:55`, `iter.any(n.next, \\(_, k) {...})`, a dummy `_` for the value when only the key matters | shown to the author |
| #36 | Excerpt, idiomatic (`@name`): `game/game.atm:52`, `emit @k :pingu.go [act=act]` | shown to the author |
| #37 | Excerpt, bad (`@name`): `screens/worldmap.atm:150`, `SAVE.worldmaps@(wm.head.short_name)`, a path still needs the parentheses | shown to the author |
| #38 | Excerpt, idiomatic (IDIOMS.md): `game/playfield.atm:31`, `loop e on :mouse.button.dn until it.button == 1 {...}` | shown to the author |
| #39 | Excerpt, bad (IDIOMS.md): `game/hud.atm:114`, `loop m on :mouse.motion until inside(r, it.x, it.y) { cam_to(m.x, m.y) }`, two names for one event | shown to the author; sent to `atmos-lang`; done in lang `9d4561a` (atmos-lang#135), applied |

## A. Decisions waiting for the author

2. **The 5 demos whose traces differ from the official game**
   (`indiana-yingwan`, `pingusworld3-pacho`, `snow1`,
   `stone-castle-timpany`, `xmas-blockblow-timpany`; `doc/fidelity.md`).
   The author approved investigating the early ones: done, 149 of 154.
   *Recommended:* investigate these 5 the same way, with the per-tick
   traces, or accept them as the limit.
4. **Expressiveness over exactness, after full fidelity** (a rule of
   the author for every port, relayed by the session `atmos-lang`): once a
   port is identical to its original on its runs, it may relax the
   exactness in favor of expressiveness, if the intent of the original
   author stays. Until then, a difference is a defect. This port is not
   there yet: 149 of the 154 demos that the official game plays give the
   same trace, 5 differ (A.2), 5 cannot be compared. An option, not a
   task. Candidates, the four places that read poorly because they copy
   the original (objects that poll every tick, the order of reactions in
   a tick by the order of spawns, the teleporter writing a position,
   clocks by arithmetic), and the floats of 32 bits.
   *Recommended:* first decide A.2 (whether to reach 154 of 154, or
   accept the 5 as the limit); then tag the faithful commit, and relax
   in small commits, each with what it gives up and gains, the outputs
   replaced in the same commit, and a document of the deliberate
   differences; never mixed with the move to `env-sdl` (B.11), which
   must keep the outputs identical. Ask the author what to relax before
   a large change.

## B. Work known and not done

0. The remote of the checkout still has the old address
   (`fsantanna/ceu-atmos-pingus`); GitHub redirects the pushes.
   Changing it (`git remote set-url origin
   https://github.com/ia-atmos/sdl-pingus`) was refused by the
   permissions of the session: the author may do it, or allow it.

1. Input configuration and controllers: the only unchecked item of the
   README; the keys of the original are fixed in `game/game.atm`.
2. Drawing offsets: the bridger (`get_look` of the original, about 25
   lines) and the center of the climber (`doc/todo.md`).
3. The worldmap: fast forward, and its border (open in `doc/todo.md`).
4. A test of blocker, faller, blocker (`doc/todo.md`, partly).
5. Fullscreen and resolution were tested only with the test driver,
   which keeps the window at 800x600: not seen on a real screen.
6. The languages in the options are in the order of their codes; the
   original sorts them by name.
7. `doc/fidelity.md`: its first table counts the solved demos of
   `tst/demos.sh` as 10; it is 12 now. Its count of solved levels names
   10.
8. `doc/bench.md`: says that fast forward is "5 ticks per frame", while
   the code runs 4 ticks per 20 ms: to check. A fast forward run by
   `PINGUS_KEYS` gave the same frames as without it: to check that the
   key of the script works.
9. The 5 demos that the official game cannot load (`crystal-10-rz`,
   `desert5-tflavel-original`, `egypts1-judit`, `hellmouth-23-rz+eriksoe`,
   `hellmouth-24-rz`): no comparison for them.
10. Language features that would help, for the compiler (sent in the
    session): guarded awaits over a pool (objects poll every tick), an
    explicit order of reactions in a tick (`:tick`/`:tock`, the order of
    spawns), a single writer of a pingu (the teleporter writes its
    position), clocks as values (timings by arithmetic), `f32` (now `ATMOS_FLOAT=32`, #18). None is
    asked; the port works around them.

## C. Reported, not acknowledged

1. The full options (commits `4280bbe` to `32a88e7`): all the options
   of the official game but its renderer; "Reset to defaults" is not in
   the official game either (commented out there).
2. The run of the port with the compiler `b3cf341`: tests, ASAN, the
   159 traces byte-identical, the benchmark unchanged (sent to the
   session `atmos-lang` and to the author).
3. The tips for a port of SuperTux (sent to the session `atmos-lang`).
4. The migration to the new rules of `pin` (`5b0f721` to `cc5fe72`),
   reported to the session `atmos-lang` (the compiler): 128 keyword changes, 15 rewrites,
   tests identical; 3 demo traces changed by the compiler `c467e60`
   and now match the official game (145 of 154).
5. The answers to env-sdl (screenshots by pixels, no test on the
   frame cap, baseline `cc5fe72`).

## D. Older items

0. Coordination, status and questions for the author (2026-11-24): the
   session `atmos` (`session_011eECohhJbc4GMpAj3yyeEo`); measurements and
   merges of the compiler stay with `atmos-lang`.
   Where to report (2026-11-18): about `pin`, the session `atmos-pin`
   (`session_011eECogSDhkVfyt7qme5DLh`), with a small program; anything
   else of the compiler, the session `atmos-lang`
   (`session_011eECogKVQ7ZzmWCoMST2wK`); the SDL environment, `env-sdl`
   (`session_011eECogZjerUUUp2BmsSRgW`).

1. `doc/fidelity.md`: the counts between 39 and 142 identical traces
   were not recorded; only the measured ones are in the table.
2. The address of the compiler: `../fsantanna/ceu-atmos` in this
   container, a checkout of `main`; `run.sh` finds it by its former
   names too.
4. Done: the scroll of the backgrounds by themselves, a live defect
   found by the compiler (`ia-atmos/lang` `3cc4855`).
5. Done, approved by the author: the workarounds `once`/`cycle` and
   `'"'` removed (`ecaa74a`); the note of the newer compiler in
   `doc/bench.md` (`30b1d97`); the three demos with an early difference
   investigated and fixed: the bomber (`intensiv-timpany`), the smasher
   (`real6`), the basher (`stone5-grumbel`).
6. Done: the move to the shared environment `ia-atmos/env-sdl`
   (`2723b34`, then `9fea770`: a scripted press moves the mouse first,
   `fps=0` is no cap), commits `e9fd322` and `b7e125f`: all tests identical
   (normal and ASAN, screenshots by pixels), the 159 demo traces
   identical. The sprite event became `:sprite.frame`.
7. Done: the compiler `ia-atmos/lang` `a4c465a` (the rules of `pin` of
   2026-11-17): one live defect it found, the item of a list pinned a
   closure over its rectangle in an inner spawn (`75e3966`); tests, ASAN,
   and the 159 demo traces identical.
8. Done: the second migration to `pin` (rules of 2026-11-17, declared
   stable; `ia-atmos/lang` `b19fd76`), commits `2e14045` to `2aa6e7b`:
   10 pins given away by their block became `val`; the options of
   `Sprite` are one table again (A.3, as recommended); `pin over` again;
   `X.in`, `:atmos`. Tests, ASAN, and the 159 demo traces identical.
3. Done: the names of the repositories (`ia-atmos/sdl-pingus`,
   `ia-atmos/lang`, the link of the plan) in `run.sh`, `README.md`,
   `doc/compiler.md`, `tools/bench/README.md`.
