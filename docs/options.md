[← Documentation](README.md)

# Options

Most of `OPTION` behaves as the PicoMite manual says. These are the ones that matter here.

## What you see

`OPTION CODEPAGE` decides how bytes 128 to 255 are printed. A terminal is Unicode and a Colour Maximite 2 screen is not, so a CMM2 program drawing boxes prints the wrong characters until this is set. `CMM2` maps them as closely as the CMM2 font allows. The others are `CP437`, `CP1252`, `MMB4L` and `NONE`.

`OPTION TAB` sets the distance of the tab stops, 4 by default. `LINE INPUT #` turns a tab it reads from a file into spaces up to the next stop, as the PicoMite and MMBasic for Windows do with a fixed 4. `INPUT$()` returns a file's tabs as they are.

`OPTION AUTOSCALE OFF` stops a simulated device's window being scaled up to fill the display. It is on by default.

## What runs

`OPTION SIMULATE` switches the whole command set to another MMBasic device. Give it on the command line with `-s`, see [Colour Maximite 2 programs](cmm2.md) and [PicoMite programs](picomite.md).

`OPTION EDITOR` picks what `EDIT` opens, see [editing programs](editor.md).

`OPTION AUDIO OFF` silences everything. Audio interrupts stop firing with it. On a PicoMite the same option assigns the audio pins, and a Colour Maximite 2 has no `OPTION AUDIO` at all.

`OPTION ESCAPE` lets string constants and quoted `DATA` take escape sequences such as `\n`. `OPTION MILLISECONDS ON` adds milliseconds to `TIME$`. `OPTION PROFILING ON` counts what the program runs and prints a report at `END`, and `LIST PROFILE` then shows it line by line. `OPTION CONTINUATION LINES ON` makes `LIST` split long lines with ` _` and joins such lines again when a program loads. All four work as on the PicoMite.

## Keeping them

Some options are kept. `OPTION EDITOR`, `OPTION AUDIO`, `OPTION AUTOSCALE`, `OPTION TAB`, `OPTION CONTINUATION LINES`, the function keys and a few more are written to `~/.mmbasic/mmbasic.options` the moment you set them and hold for every later start. `OPTION CODEPAGE`, `OPTION SIMULATE` and `OPTION PROFILING` last until you quit. `OPTION ESCAPE` and `OPTION MILLISECONDS` belong to the program that sets them and are cleared by the next `RUN`.

```basic
OPTION LIST
OPTION RESET EDITOR
OPTION RESET ALL
OPTION SAVE "myoptions"
OPTION LOAD "myoptions"
```

`OPTION SAVE` writes the kept options you changed into a file of your own, and `OPTION LOAD` reads it back. `OPTION RESET` needs an option name or `ALL`.
