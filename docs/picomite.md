[← Documentation](README.md)

# PicoMite programs

Most of the PicoMite's language works in every mode. Commands such as `DO UNTIL`, `TRIM$` or the `ARRAY` commands need nothing special. What a PicoMite program needs `-s` for is the screen and the graphics that belong to the device.

```sh
mmbasic -s PicoMiteVGA prog.bas
```

The devices are `PicoMiteVGA`, `PicoMiteHDMI`, `PicoMiteVGAUSB`, `PicoCalc` and `GameMite`, see [Running MMB4M](running.md). Give the device on the command line rather than with `OPTION SIMULATE` inside the program, so the program runs as that device from its first line.

## What comes back

`MODE`, which opens a window: `MODE 1` gives 640 by 480. The drawing commands, `FRAMEBUFFER`, `PLAY` and `KEYDOWN`.

`MM.DEVICE$` and `MM.INFO$(DEVICE)` report the simulated device, which is what most programs check. `MM.INFO$(DEVICE X)` still reports the real one. `PicoCalc` reports `PicoMite` as the device and `PicoCalc` as the platform.

## What does not

`PAGE`, which a PicoMite does not have either. A program that uses it was written for a Colour Maximite 2.

The `GAMEPAD` commands. Controllers are read with `DEVICE GAMEPAD` here, see [What MMB4M adds](commands.md).

Everything a Mac does differently in general, from pins and flash memory to `CSUB` and timing, is on [Differences](differences.md).
