[← MMB4M](../README.md)

# Known problems in 0.1.0

Defects in this version, and what has not been tested. For the things a Mac will never do, see [what MMB4M cannot do](limits.md).

## Tested and untested

The Apple Silicon and the Intel build both start and run programs. A graphics program using `GRAPHICS WINDOW`, `GRAPHICS BUFFER` and `GRAPHICS COPY` runs. A large CMM2 program using `MODE` and `PAGE WRITE` runs when started with `-s`. Eight checks run before a release. They cover the source being unchanged from its origin, the patches, the version numbers, the licence conditions, the wording, privacy, the binaries being what they claim, and the interpreter running the test programs.

Untested: serial ports, gamepads, and VS Code as editor. The Intel build has only run under Rosetta on an Apple Silicon machine, not on an Intel Mac. The internal editor has been driven by a script, not used by hand.

## OPTION SIMULATE faults when the program changes mode

Switching device from inside a running program dies at the next graphics command:

```
Error in line 2: src/common/events.c:157 internal fault
```

Switching at runtime tears down the window and builds it again while SDL still references the old one, so the interpreter cannot find the window by its id.

Start with `-s "Colour Maximite 2"` and leave the `OPTION SIMULATE` line out.

Reproduced on 2026-09-04: two programs with the identical `MODE 1, 8` line and the same binary, one faulting, one running.
