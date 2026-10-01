[← Documentation](README.md)

# Differences

Most MMBasic programs run on a Mac as they are. This page lists what works differently and what a Mac cannot do. How large a program can get is on [Limits](limits.md), and defects of this version are on [Known problems](known-issues.md).

## Windows instead of a screen

A Mac has no screen modes. A program opens its own window with `GRAPHICS WINDOW` and draws into it, see [Graphics](graphics.md). `MODE`, `PAGE` and `FRAMEBUFFER` answer `Unsupported on current device/platform`. Started as a Colour Maximite 2, `MODE` and `PAGE` come back, and started as a PicoMite, `MODE` and `FRAMEBUFFER` do. See [Colour Maximite 2 programs](cmm2.md) and [PicoMite programs](picomite.md).

`PRINT` always writes to the terminal, even with a window open and while simulating another device. `TEXT` draws text into the window.

Sound and colour go through SDL, so they are close to the real devices but not identical.

## Programs are files

There is no program in flash memory. The current program is a file on disk, and `EDIT` edits that file, see [Editing programs](editor.md). `SAVE "copy.bas"` writes a copy of it. Inside a running program `LOAD` needs `,R`, which loads another program and runs it.

## Hardware

A Mac has no pins and no flash memory of the microcontroller kind. `SETPIN`, `PIN`, `PULSE` and `FLASH` answer `Unsupported on current device/platform`. The PicoMite's commands for buses, sensors, LEDs and displays, such as `I2C`, `SPI` or `WS2812`, do not exist here at all.

Game controllers connected to the Mac are read with `DEVICE GAMEPAD` instead of the PicoMite's `GAMEPAD`, see [What MMB4M adds](commands.md).

`CSUB` and `CFUNCTION` blocks load and are skipped. They hold machine code for the processor of the device they were written for, so calling one stops the program with `Unsupported on current device/platform`.

## Serial ports

MMBasic offers rates up to 4000000 baud. macOS defines nothing above 230400, so a higher rate fails rather than running slowly. Ports are named `/dev/cu.*`, which `ls /dev/cu.*` lists. Serial ports have not been tried on a Mac yet.

## Timing

macOS is not a real-time system, so `PAUSE`, `SETTICK` and anything else on the clock vary more than on a microcontroller. A program tuned to the frame timing of a Colour Maximite 2 will not keep the same rhythm. `SETTICK FAST` does not exist here.
