[← MMB4M](../README.md)

# What MMB4M cannot do

What a Colour Maximite 2 or a PicoMite does and MMB4M does not.

For defects in this version rather than permanent limits, see [known problems](known-issues.md).

## Commands that are not there

They report `Unsupported on current device/platform` rather than failing to parse, so a program gets as far as the line before it stops.

| Not available | Instead |
| --- | --- |
| `MODE`, `PAGE` | [the `GRAPHICS` commands](graphics.md), or start as a [simulated CMM2](cmm2.md) |
| `FRAMEBUFFER` | [the `GRAPHICS` commands](graphics.md). Colour Maximite 2 mode does not bring it back |
| `SETPIN`, `PIN`, `PULSE`, `FLASH` | nothing. Microcontroller hardware |
| `GAMEPAD` | `DEVICE GAMEPAD`. The function `GAMEPAD()` is unsupported as well |

`SAVE "prog.bas"` is not there either, because the file on disk is the program. It answers `Unknown SAVE subcommand`, since `SAVE IMAGE` does exist.

`CFUNCTION` and `DUMMY` do not exist at all.

## Serial stops at 230400 baud

MMBasic offers rates up to 4000000. macOS defines nothing above `B230400`, so the higher ones do not exist here and setting one fails rather than running slowly.

macOS names serial ports `/dev/cu.*`, not `/dev/ttyUSB0`; `ls /dev/cu.*` lists them. Serial support is described by its author as a work in progress and as slow and unreliable in places. Untested on a Mac.

## Timing drifts

macOS is not a real-time system, so `PAUSE`, `SETTICK` and anything else on the clock vary more than on a microcontroller. A program tuned to the frame timing of a Colour Maximite 2 will not keep the same rhythm. `SETTICK` works, `SETTICK FAST` does not.

## Ceilings

Paths stop at 255 characters. Program code is capped at 0.5 MB and variables and other RAM at 1 MB. These are MMBasic's limits, not the Mac's.
