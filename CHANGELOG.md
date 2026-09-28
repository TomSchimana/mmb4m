# Changelog

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
