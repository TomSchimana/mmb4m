[← MMB4M](../README.md)

# What exists here and not on a PicoMite or a Colour Maximite 2

What a Mac adds: the terminal, the operating system, USB controllers, and the things MMBasic reports about itself. Where a device is named below, the claim was checked against that device's own manual. The [MMBasic for Linux README](https://github.com/thwill1000/mmb4l#readme) carries the full syntax for most of it, though it is behind the interpreter in places.

## The console

```basic
CONSOLE FOREGROUND BRIGHT YELLOW
PRINT "warning"
CONSOLE RESET
```

`CONSOLE FOREGROUND`, `BACKGROUND`, `INVERSE` and `RESET` set colour. Eight colours, 0 to 7, black to white, and `BRIGHT` gives the second eight.

`CONSOLE SETCURSOR`, `GETCURSOR`, `HOME` and `HIDECURSOR` move and hide the cursor, in character coordinates from the top left. `CONSOLE GETSIZE` reads the terminal size into two variables. A terminal has no fixed size, unlike the screen of either device.

`CONSOLE CLEAR` clears and homes the cursor. `CONSOLE BELL` rings the bell.

`PRINT @(x, y) expr` prints at a position counted in pixels of a nominal 8x12 character cell, `PRINT @C(x, y)` at a position counted in characters. Both coordinates are obligatory. Both devices take `@(x [, y])` and `@(x, [y], mode)`, leaving the cursor on the current line when `y` is left out; neither form exists here.

## The operating system

```basic
SYSTEM "ls -l", output$
SETENV "MY_VAR", "value"
PRINT MM.INFO$(ENVVAR "HOME")
```

`SYSTEM cmd$` runs a shell command and can capture its output into a string or a long string. `SETENV` and `SYSTEM GETENV` read and write environment variables. At the prompt `!cmd` is short for `SYSTEM`.

Each `SYSTEM` runs in its own process, so anything it changes about the working directory is gone when it ends. `!cd` is turned into `CHDIR` for that reason.

## Ending a program

`QUIT [code%]` leaves MMB4M and hands the code to the shell, 0 to 255. `END [code%]` ends the program and returns to the prompt, where `MM.INFO(EXITCODE)` holds the code. Run non-interactively, `END` behaves like `QUIT`.

You get 0 for a clean end, 130 after Ctrl-C and 1 after an unhandled error.

`ERROR [msg$ [, errno%]]` raises an error deliberately. `MM.ERRMSG$` and `MM.ERRNO` carry it. Numbers 1 to 255 are C library `errno` values and 256 means unclassified. Everything from 257 to 1023 can change between releases, so use 1024 upwards for your own.

## Game controllers

```basic
DEVICE GAMEPAD OPEN 1, on_button
IF DEVICE(GAMEPAD 1, B) AND &b1 THEN PRINT "R"
```

Up to four controllers, ids 1 to 4, read through SDL. `DEVICE GAMEPAD OPEN`, `CLOSE`, `RUMBLE`, `RUMBLE TRIGGERS`, `LED`, `INTERRUPT ENABLE` and `INTERRUPT DISABLE`, and the function `DEVICE(GAMEPAD id, funct)` where `B` returns a bitmap of the digital buttons and `LX`, `LY`, `RX`, `RY`, `L` and `R` the analog axes and triggers.

A PicoMite reads USB controllers with its own `GAMEPAD` commands and the same `DEVICE(GAMEPAD ...)` function, with different subcommands: `COLOUR` and `HAPTIC` there against `LED` and `RUMBLE` here. A Colour Maximite 2 has neither and reads Wii Classic controllers over I2C.

`MM.INFO$(GAMEPAD id%)` gives SDL's identification string for an attached controller, and the empty string when none is attached.

If opening reports a `Gamepad error` about a missing mapping, SDL does not know that controller. Generate a mapping with the [gamepad tool](https://www.generalarcade.com/gamepadtool/) and put it in `SDL_GAMECONTROLLERCONFIG` before starting MMB4M.

Untested on a Mac.

## Smaller things

`CHR$(UTF8 ...)` builds UTF-8 characters, which a terminal can show and neither device's screen can. `RUN` takes expressions where other platforms take literals. `PEEK(DATAPTR)` and `POKE DATAPTR` save and restore the `DATA` read position. `XMODEM` transfers over an open serial port; on both devices it goes over the console instead, which is what this build cannot do.

## What the interpreter knows about itself

`MM.INFO(ARCH)` answers `macOS arm64` or `macOS x86_64`. `MM.INFO$(DEVICE)` answers `MMB4L`, or the simulated device when started with `-s`. `MM.INFO$(DEVICE X)` gives the real device even while simulating another one.

`MM.VER` is an integer here, not a float, so a comparison against a decimal fails. It carries MMB4M's own version, as `MM.INFO(VERSION)` does, not the version of the MMBasic for Linux release underneath; the changelog records that.

`MM.INFO$(ENVVAR name$)` reads an environment variable and `MM.INFO$(OPTION x)` any option's current value.

### The program and the process

`MM.INFO$(CURRENT)` gives the full path of the running program, and `NONE` after `NEW`.

`MM.INFO$(LINE)` gives the line being executed as `number,path`, not a number on its own, and `UNKNOWN` where there is no line.

`MM.INFO(CALLDEPTH)` counts how deep the call nesting is, 0 outside any `SUB` or `FUNCTION`.

`MM.INFO(PID)` is the process id the shell sees. `MM.INFO(CPUTIME)` is the CPU time this process has used, in nanoseconds.

### Errors

`MM.INFO(ERRNO)` and `MM.INFO$(ERRMSG)` carry the last error, and `ERRMSG` includes the `Error in line n:` prefix. With a number after it, `MM.INFO$(ERRMSG 2)` gives the standard message for that error number, for 1 to 255 the C library texts.

### Files

`MM.INFO(EXISTS path$)` is 1 if a file, directory or device of that name exists. `MM.INFO(EXISTS SYMLINK path$)` asks whether it is a symbolic link, which neither device has. Both spell the first one `EXISTS FILE` or `EXISTS DIR`.

### Where PRINT left the cursor

`MM.INFO(HPOS)` and `MM.INFO(VPOS)` give the position after the last `PRINT`, counted in pixels of a nominal 8x12 character cell. `MM.INFO(HPOS C)` and `MM.INFO(VPOS C)` give characters. Drawing does not change them, see [graphics](graphics.md).
