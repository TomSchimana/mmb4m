[← MMB4M](../README.md)

# PicoMite programs

Start the interpreter as a PicoMite and the screen modes and graphics a PicoMite program expects come back.

```sh
mmbasic -s PicoMiteVGA prog.bas
```

The devices are `PicoMiteVGA`, `PicoMiteHDMI`, `PicoMiteVGAUSB`, `PicoCalc` and `Game*Mite`, the last one also as `GameMite`. For a Colour Maximite 2 see [Colour Maximite 2 programs](cmm2.md).

Do not set `OPTION SIMULATE` inside the program. It faults at the next graphics command, see [known problems](known-issues.md).

## What comes back

`MODE`, which opens a window: `MODE 1` gives 640 by 480.

The drawing commands, `FRAMEBUFFER`, `PLAY` and `KEYDOWN`.

`MM.DEVICE$` and `MM.INFO$(DEVICE)` report the simulated device, which is what most programs branch on. `MM.INFO$(DEVICE X)` returns the real device. `PicoCalc` reports `PicoMite` as the device and `PicoCalc` as the platform.

## What does not

`PAGE`. A PicoMite VGA has no pages either; a program that uses them is written for a Colour Maximite 2.

`SETPIN`, `PIN`, `PULSE` and `FLASH` address microcontroller hardware. `FLASH` answers `Unimplemented` rather than refusing outright.

The `GAMEPAD` commands and the `GAMEPAD()` function. Controllers are read with `DEVICE GAMEPAD` here, see [what exists here](commands.md).

`PRINT` writes to the terminal, never to a graphics surface, simulation included. `TEXT` draws.

The program in flash memory does not exist here, and `EDIT` works on files only. See [editing programs](editor.md).

Timing drifts. macOS is not a real-time system, so `PAUSE` and `SETTICK` vary more than on the hardware, and `SETTICK FAST` is not there at all.

`MM.INFO(CPUSPEED)` and `MM.INFO$(DRIVE)` crash the interpreter in this mode, see [known problems](known-issues.md).
