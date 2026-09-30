# Changelog

## 0.1.4, 2026-09-30

Fixes twelve defects: two hangs and crashes, wrong results at the edges of the number range, and names that silently read as zero.

- A program with all ten file numbers open no longer hangs when it ends. Found in MMB4M 0.1.3; the defect is in MMB4L v0.8-alpha.1, commit 8e98d84
- A program just under the 512 KB program memory says `Program too long` instead of crashing or reporting an internal fault. Found in MMB4M 0.1.3; the defect is in MMB4L v0.8-alpha.1, commit 8e98d84
- `MATH M_INVERSE` can be repeated without running out of memory, and inverts a 1x1 array. From PicoMite V6.04.00, commit c994e92
- `MAX(-1E300, -2E300)` is `-1e+300`, and `MIN` likewise; both used to stop at ±3.4e+38. From PicoMite V6.04.00, commit 50c9aa0
- `ABS` accepts floats of 2^63 and more instead of raising `Number too large`. From PicoMite V6.04.00, commit 16d0413
- Infinity and NaN print as `INF`, `-INF` and `NAN` instead of `0e+2147483647`. From PicoMite V6.04.00, commit a40112b
- `INT(1E30)`, `FIX(-1E30)`, NaN and 2^63 raise `Number too large` when turned into an integer instead of giving a wrong number. From PicoMite V6.04.00, commit a40112b; the check on 32-bit values found in MMB4M 0.1.3, a defect in MMB4L v0.8-alpha.1, commit 8e98d84
- `RANDOMIZE` without an argument seeds from the clock instead of being a syntax error. From PicoMite V6.04.00, commit 1457087
- `STATIC` belongs to the SUB it is written in; before, a call to another SUB just above it made the two share the variable, and an interrupt SUB got a second one. From PicoMite V6.04.00, commit b0f448e
- `STATIC` in a routine reached by `GOSUB` outside any SUB raises `Invalid here`, as on the PicoMite. From PicoMite V6.04.00, commit b0f448e
- `MM.FONTHEIGHT`, `MM.FONTWIDTH`, `MM.HPOS`, `MM.VPOS`, `MM.WIDTH` and `MM.HEIGHT` exist, as on the PicoMite and the Colour Maximite 2; they used to be variables worth 0. Names from PicoMite V6.04.00, values as `MM.INFO()` gives them
- `MATH CRC8`, `CRC12`, `CRC16` and `CRC32` honour their reverse-output argument. From PicoMite V6.03.02b2
- `MATH CHI` and `CHI_P` refuse more than 50 degrees of freedom instead of reading past their table, and count them right under `OPTION BASE 1`. From PicoMite V6.03.02b2

## 0.1.3, 2026-09-28

Fixes PLAY TONE, and withdraws two commands that were never finished. Now built from MMBasic for Linux 0.8 alpha 1, branch `main`, commit `8e98d84`.

- `PLAY TONE` plays the right channel, at its own frequency. Before, `PLAY TONE 440, 660` sounded 440 Hz on the left and nothing on the right
- `MM.INFO(CURRENT FUNCTION)` and `LIST CALLS` are gone. Both were unfinished in 0.1.0 to 0.1.2, and upstream withdrew them for this release
- fonts 8 and 9 from the PicoMite
- sound is handed to macOS in blocks of 1024 samples instead of one at a time, upstream's fix against crackling

## 0.1.2, 2026-09-06

Fixes four defects: two crashes, one command that wrote a file it should not, and one refusal that said nothing useful.

- `OPTION SIMULATE` inside a running program no longer faults at the next graphics command
- `MM.INFO$(SDCARD)`, `MM.INFO(CPUSPEED)` and `MM.INFO$(DRIVE)` no longer end the process with a segmentation fault
- `EDIT` after an error at the prompt no longer creates and edits a file called `<PROMPT>`
- `MODE` and `PAGE` name the command line that brings them back: `mmbasic -s CMM2` or `mmbasic -s PicoMiteVGA`

## 0.1.1, 2026-09-05

Fixes reading from the console. In 0.1.0 both commands printed their prompt and then stopped before reading a character.

- `INPUT` and `LINE INPUT` read the console again, instead of failing with `Invalid file number`

## 0.1.0, 2026-09-05

First version, for Apple Silicon and Intel, one zip per architecture holding the single file `mmbasic`. Built from MMBasic for Linux, branch `develop-v0.8-8`, commit `529fded`.

- builds and runs on macOS, Apple Silicon and Intel
- `PATH_MAX` comes from the macOS header, so paths stop at 1024 characters
- serial baud rates above 230400 are gone; macOS does not have them
- `EDIT` opens MMBasic's own editor by default; `OPTION EDITOR` still selects nano, VS Code or any other
- the port's copyright line in the startup banner, under the original three
- the banner, `--version` and `MM.INFO(VERSION)` report the port's own version and `macOS arm64` or `macOS x86_64`
