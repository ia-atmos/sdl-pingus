# What the port found in the compiler

The plan listed what the port might need from the compiler
(`ia-atmos/lang`, then named `ceu-atmos`). What the
work found, in the order it was found:

## Worked around

- **A field named as a keyword in a constructor**: `[loop=false]` was
  a syntax error, while `d.loop` and data files with `loop = false`
  worked. The options of `Sprite` were `once` and `cycle`. Fixed in
  the compiler (`b3cf341`); the option is `loop` again, as the data.
- **A native literal with one backquote is one line**: a call that
  spans lines must be on one line, or use three backquotes.
- **A nested `func` is a global**: `func f` inside a task declares a
  variable of the main chunk, as the manual says, so two sprites shared
  their `advance`, and a closure over a pinned table escaped. Nested
  functions are `val func`.
- **`emit @(:parent)` from a task in a pool reaches the pool**, the
  siblings of the task, and not the task that holds the pool. The
  buttons of the actions rely on it (they select and unselect each
  other); the dots of the worldmap receive the view as an argument, and
  emit to it (`emit @v`). The manual says only "parent task": the case
  of pools may deserve a line there.
- **`await x` of a variable** needs the value form, `await(x)`.
- **`loop n { }`** is not a counted loop: `loop _ in n { }`.
- **A string with an escaped delimiter** (`"a\"b"`) was a syntax
  error; `'"'` was used instead. Fixed in the compiler (`b3cf341`).
- **Iterating a tagged table** gives the field `tag` among the items:
  an index of data (a tagged vector) is iterated by position.

## Found by the compiler

- **A spawn that dies at the end of its block** (`ia-atmos/lang`
  `3cc4855`, a new error: `invalid spawn : the task is aborted at the
  end of its block`): the scroll of a background by itself was spawned
  inside an `if`, and was aborted at its `}`; the backgrounds that
  scroll by themselves (in 66 levels) never moved, and no trace saw
  it, since a background acts on no pingu. The test is now inside the
  spawn, and the scroll is a float that wraps by one size, as the
  original.

## The new rules of `pin` (`ia-atmos/lang` `98ad03a`, 2026-11-15)

Every dynamic value belongs to a block; `val` and `var` only refer to a
value that has an owner, and the first declaration of a fresh value is
`pin`. The migration of the port, with the traces and screenshots of
the tests identical. Of the 159 traces of the demos, 156 are as before;
3 changed, and now match the official game: a bisection of
`aliens6-phil` on the pre-migration port puts the change in the
compiler, at `c467e60` (a loop over a pool tolerates the aborts of its
tasks), not in the migration (see `fidelity.md`):

- 128 lines changed only from `val`/`var` to `pin`.
- 15 places rewritten, by kind:
  - a cache or a list owns the value, which is completed in place
    (`res_desc`, `level_flatten`, `levelsets`, the paths of the
    worldmap): the fresh value is stored first, and then referred to;
  - a variable that received fresh values became a pinned table, or a
    field of one (`ACTIONS`, filled by fields; `LANG.texts`; the
    slowest file of `tools/check.atm`);
  - a value that is sometimes fresh and sometimes owned (`x || []`):
    the options of `Sprite` are read into locals from the parameter;
    the defaults of the save, of a prefab position and of a color
    became scalars; the overrides of a prefab always make a copy;
  - a task owns what it receives: the rectangle of an item of a list
    is built in the spawn;
  - a function gives a value that is either loaded or a default
    (`save_load`);
  - a fresh result that was indexed and dropped (`levelsets()@i`) is
    pinned once.
- Two `[y]` wrappers in the walker and the basher, which made a
  result of a loop a table, became the number (0 is true in Atmos):
  one table less per pingu per tick.

## Measured

- **Speed**: a task wake costs about 55 ns (2000 tasks, 1000 events,
  0.11 s). The game with 100 pingus and fast forward runs at 500 frames
  per second with the software renderer, which takes most of the time.
- **`X.load`**: 2 ms for the largest level (334 KB). No cache needed.
- **Build time**: 24 s for the whole game (one C file, `-O2`). The tests
  build the game once. A faster build (`-O1`, or separate compilation
  of the libraries in Atmos) would help the edit-run loop.
- **The address sanitizer** finds no error and no leak in the tests.

## Not needed

- Name clashes of `include`: none in 60 files, with prefixes by unit
  (`wm_`, `basher_`, `pg.`).
- The tree of tasks in the error traces: the errors were found with the
  traces of calls, and the trace of actions of the game (`PINGUS_TRACE`).
