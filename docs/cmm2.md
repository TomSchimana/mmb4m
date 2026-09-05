[← MMB4M](../README.md)

# Colour Maximite 2 programs

Start the interpreter as a Colour Maximite 2 and most of what a CMM2 program expects comes back.

```sh
mmbasic -s "Colour Maximite 2" prog.bas
```

`CMM2` works as a short form. The other devices are `PicoMiteVGA`, `PicoMiteHDMI`, `PicoMiteVGAUSB`, `PicoCalc`, `Game*Mite` and `MMBasic for Windows`.

Do not set `OPTION SIMULATE` inside the program. It faults at the next graphics command, see [known problems](known-issues.md).

## What comes back

`MODE`, `PAGE COPY`, `PAGE SCROLL`, `PAGE WRITE`, `CONTROLLER CLASSIC OPEN` and `CLOSE`, and the function `CLASSIC()`.

`MM.DEVICE$` and `MM.INFO$(DEVICE)` report the simulated device, which is what most programs branch on. `MM.INFO$(DEVICE X)` returns the real device.

USB controllers appear as Wii Classic controllers on I2C in the order a CMM2 program expects: USB1 as I2C3, USB2 as I2C1, USB3 as I2C2.

The window is scaled up as far as your display allows. `OPTION AUTOSCALE OFF` stops that.

## What does not

`PRINT` writes to the terminal, never to a graphics surface, simulation included. `TEXT` draws.

`SETPIN`, `PIN`, `PULSE` and `FLASH` are not available; they address microcontroller pins.

Timing drifts. macOS is not a real-time system, so `PAUSE` and `SETTICK` vary more than on a CMM2, and a program tuned to hardware frame timing runs differently. Sound and colour go through SDL2 and are close, not identical.

## One thing MMBasic is stricter about

`DIM x(0)` for a single-element array is rejected with `Dimensions`.
