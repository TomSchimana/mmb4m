[← Documentation](README.md)

# Known problems

Defects in the current version, and what has not been tested. What a Mac does differently on purpose is on [Differences](differences.md).

## Defects

- Text pasted into the editor (`EDIT`) loses characters: the editor throws away what arrives while it redraws a line. Typing by hand is not affected. Paste a whole program with `AUTOSAVE "name.bas"` instead, which takes it without loss; F1 ends it. Found in MMB4M 0.1.4; the defect is in MMB4L v0.8-alpha.1, commit 8e98d84, and earlier MMB4M versions have it too

- `TAB()` counts a wide character, such as a Chinese, Japanese or Korean character or an emoji, as one column, while a terminal shows it in two, so what follows lands one column further right for each. Letters such as umlauts (ä, ö, ü) count right. Found in MMB4M 0.2.1

- `DIM s AS STRING LENGTH 20` gives `s` the full 255 characters: a `LENGTH` after the type has no effect, as on the PicoMite, the Colour Maximite 2 and MMBasic for Windows, whose programs write it that way. Write `DIM s LENGTH 20 AS STRING` for the limit to apply

- `MATH SCALE` from an integer array into an integer array with a fractional scale rounds each result to the nearest integer, where the PicoMite and the Colour Maximite 2 cut the fraction off: `MATH SCALE a%(), 0.5, b%()` makes 31 into 16 here and into 15 there. From a float array all three round. Found in MMB4M 0.2.1

## Tested and untested

The Apple Silicon and the Intel build both start and run programs. A graphics program using `GRAPHICS WINDOW`, `GRAPHICS BUFFER` and `GRAPHICS COPY` runs. A large CMM2 program using `MODE` and `PAGE WRITE` runs when started with `-s`. Eleven checks run before a release. They cover the source being unchanged from its origin, the patches, the version numbers, the licence conditions, the wording, privacy, the binaries being what they claim, the interpreter running the test programs, MMBasic for Linux's own test suite, its behaviour in a real terminal, and the sound it produces.

Untested: serial ports, gamepads, and VS Code as editor. The internal editor has been driven by a script, not used by hand.
