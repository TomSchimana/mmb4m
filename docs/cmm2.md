[← Documentation](README.md)

# Colour Maximite 2 programs

Start the interpreter as a Colour Maximite 2 and most of what a CMM2 program expects comes back.

```sh
mmbasic -s "Colour Maximite 2" prog.bas
```

`CMM2` works as a short form, and [Running MMB4M](running.md) lists every device. Give the device on the command line rather than with `OPTION SIMULATE` inside the program, so the program runs as that device from its first line.

## What comes back

`MODE`, `PAGE COPY`, `PAGE SCROLL` and `PAGE WRITE`, `CONTROLLER CLASSIC OPEN` and `CLOSE`, and the function `CLASSIC()`.

`MM.DEVICE$` and `MM.INFO$(DEVICE)` report the simulated device, which is what most programs check. `MM.INFO$(DEVICE X)` still reports the real one.

USB controllers appear as Wii Classic controllers on I2C, in the order a CMM2 program expects: USB1 as I2C3, USB2 as I2C1, USB3 as I2C2.

The window is scaled up as far as your display allows. `OPTION AUTOSCALE OFF` stops that.

A few things of the Colour Maximite 2 work in every mode, without `-s`. `CFUNCTION` blocks load, `>>>` shifts right keeping the sign, `MM.POS` gives the cursor's column, a `SUB MM.PROMPT` prints the prompt, and `AUTOSAVE N` takes a program without echoing it.

## What does not

`FRAMEBUFFER` stays unavailable in this mode, although a Colour Maximite 2 has it. Box-drawing characters need `OPTION CODEPAGE CMM2`, see [Options](options.md). `DIM x(0)` for a single-element array is rejected with `Dimensions`.

Everything a Mac does differently in general, from `PRINT` to `CSUB` and timing, is on [Differences](differences.md).
