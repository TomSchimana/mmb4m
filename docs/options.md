[← MMB4M](../README.md)

# Options

Most of `OPTION` behaves as the PicoMite manual says. These are the ones that matter here.

## What you see

`OPTION CODEPAGE` decides how bytes 128 to 255 are printed. A terminal is Unicode and a Colour Maximite 2 screen is not, so a CMM2 program drawing boxes prints the wrong characters until this is set. `CMM2` maps them as closely as the CMM2 font allows. The others are `CP437`, `CP1252`, `MMB4L` and `NONE`.

`OPTION AUTOSCALE OFF` stops a simulated device's window being scaled up to fill the display. It is on by default.

## What runs

`OPTION SIMULATE` switches the whole command set to another MMBasic device. Set it on the command line with `-s`, never from inside a running program, see [Colour Maximite 2 programs](cmm2.md).

`OPTION EDITOR` picks what `EDIT` opens, see [editing programs](editor.md).

`OPTION AUDIO OFF` silences everything. Audio interrupts stop firing with it.

## Keeping them

Two kinds. `OPTION EDITOR`, `OPTION AUDIO`, `OPTION AUTOSCALE`, `OPTION TAB`, the function keys and a few more are kept: setting one writes `~/.mmbasic/mmbasic.options` at once, and it holds for every later start. `OPTION CODEPAGE` and `OPTION SIMULATE` are not kept and last for the session.

```basic
OPTION LIST
OPTION RESET EDITOR
OPTION RESET ALL
OPTION SAVE "myoptions"
OPTION LOAD "myoptions"
```

`OPTION SAVE` writes the kept options you changed into a file of your own, and `OPTION LOAD` reads it back. `OPTION RESET` needs an option name or `ALL`.
