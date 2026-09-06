[← MMB4M](../README.md)

# Known problems in 0.1.1

Defects in this version, and what has not been tested. For the things a Mac will never do, see [what MMB4M cannot do](limits.md).

## Tested and untested

The Apple Silicon and the Intel build both start and run programs. A graphics program using `GRAPHICS WINDOW`, `GRAPHICS BUFFER` and `GRAPHICS COPY` runs. A large CMM2 program using `MODE` and `PAGE WRITE` runs when started with `-s`. Ten checks run before a release. They cover the source being unchanged from its origin, the patches, the version numbers, the licence conditions, the wording, privacy, the binaries being what they claim, the interpreter running the test programs, its behaviour in a real terminal, and the sound it produces.

Untested: serial ports, gamepads, and VS Code as editor. The Intel build has only run under Rosetta on an Apple Silicon machine, not on an Intel Mac. The internal editor has been driven by a script, not used by hand.

## Three MM.INFO subfunctions crash the interpreter

`MM.INFO$(SDCARD)` ends the process with a segmentation fault, no message, on every device. `MM.INFO(CPUSPEED)` and `MM.INFO$(DRIVE)` do the same while simulating a PicoMite, a PicoCalc or a Game*Mite. Under `-s "Colour Maximite 2"`, `-s MMB4W` and with no simulation those two answer `Unsupported on current device/platform` and are harmless.

Nothing else needs them: no MMBasic program running on a Mac has a CPU speed, a drive letter or an SD card to ask about.

## OPTION SIMULATE faults when the program changes mode

Switching device from inside a running program dies at the next graphics command:

```
Error in line 2: src/common/events.c:157 internal fault
```

Switching at runtime tears down the window and builds it again while SDL still references the old one, so the interpreter cannot find the window by its id.

Start with `-s "Colour Maximite 2"` and leave the `OPTION SIMULATE` line out.

Reproduced on 2026-09-04: two programs with the identical `MODE 1, 8` line and the same binary, one faulting, one running.
