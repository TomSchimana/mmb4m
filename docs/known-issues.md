[← MMB4M](../README.md)

# Known problems

Defects in the current version, and what has not been tested. For the things a Mac will never do, see [what MMB4M cannot do](limits.md).

## Defects

- Text pasted into the editor (`EDIT`) loses characters: the editor throws away what arrives while it redraws a line. Typing by hand is not affected. Paste a whole program with `AUTOSAVE "name.bas"` instead, which takes it without loss; F1 ends it. Found in MMB4M 0.1.4; the defect is in MMB4L v0.8-alpha.1, commit 8e98d84, and earlier MMB4M versions have it too

## Tested and untested

The Apple Silicon and the Intel build both start and run programs. A graphics program using `GRAPHICS WINDOW`, `GRAPHICS BUFFER` and `GRAPHICS COPY` runs. A large CMM2 program using `MODE` and `PAGE WRITE` runs when started with `-s`. Ten checks run before a release. They cover the source being unchanged from its origin, the patches, the version numbers, the licence conditions, the wording, privacy, the binaries being what they claim, the interpreter running the test programs, its behaviour in a real terminal, and the sound it produces.

Untested: serial ports, gamepads, and VS Code as editor. The internal editor has been driven by a script, not used by hand.
