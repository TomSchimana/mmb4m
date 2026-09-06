# Changelog

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
