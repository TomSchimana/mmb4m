[← MMB4M](../README.md)

# What MMB4M cannot do

What a Colour Maximite 2 or a PicoMite does and MMB4M does not.

For defects in this version rather than permanent limits, see [known problems](known-issues.md).

## Commands that are not there

They report `Unsupported on current device/platform` rather than failing to parse, so a program gets as far as the line before it stops.

| Not available | Instead |
| --- | --- |
| `MODE`, `PAGE` | [the `GRAPHICS` commands](graphics.md), or start as a [simulated CMM2](cmm2.md). `MODE` also comes back as a [simulated PicoMite](picomite.md), `PAGE` does not |
| `FRAMEBUFFER` | [the `GRAPHICS` commands](graphics.md). Colour Maximite 2 mode does not bring it back, [PicoMite mode](picomite.md) does |
| `SETPIN`, `PIN`, `PULSE`, `FLASH` | nothing. Microcontroller hardware |
| `GAMEPAD` | `DEVICE GAMEPAD`. The function `GAMEPAD()` is unsupported as well |

`CSUB` and `CFUNCTION` blocks load and are skipped, but they hold machine code for the device they were written for, so calling one stops the program with `Unsupported on current device/platform`. `DUMMY` does not exist.

## Serial stops at 230400 baud

MMBasic offers rates up to 4000000. macOS defines nothing above `B230400`, so the higher ones do not exist here and setting one fails rather than running slowly.

macOS names serial ports `/dev/cu.*`, not `/dev/ttyUSB0`; `ls /dev/cu.*` lists them. Serial support is described by its author as a work in progress and as slow and unreliable in places. Untested on a Mac.

## Timing drifts

macOS is not a real-time system, so `PAUSE`, `SETTICK` and anything else on the clock vary more than on a microcontroller. A program tuned to the frame timing of a Colour Maximite 2 will not keep the same rhythm. `SETTICK` works, `SETTICK FAST` does not.

## Ceilings

Paths stop at 255 characters. Program code is capped at 1 MB and variables, arrays and strings at 128 MB, as in MMBasic for Windows. These are MMBasic's limits, not the Mac's.

| | MMB4M |
| --- | --- |
| upper bound of one array dimension | 2147483647 |
| variables | 2048 |
| nested `FOR` and `DO` loops | 128 each |
| open files | 128 |
| entries `FILES` lists | 2048 |
| `#DEFINE` entries | 256 |
| sprite layers | 10 |

Temporary memory, for string operations and the arguments of a `SUB` or `FUNCTION` call, is taken from the bottom of the heap, and variables and arrays from the top, as on the PicoMite. A string parameter or a `LOCAL` variable is a variable, so a call that has one still looks for memory past every array, and runs noticeably slower beside arrays of many megabytes.
