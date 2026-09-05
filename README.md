# MMBasic for MacOS (MMB4M)

MMBasic is Geoff Graham's and Peter Mather's BASIC for the Colour Maximite and the PicoMite. Thomas Hugo Williams ported it to Linux as [MMBasic for Linux](https://github.com/thwill1000/mmb4l). MMB4M takes that port and builds it for the Mac.

## Install

Download the zip for your Mac from the [latest release](https://github.com/TomSchimana/mmb4m/releases/latest):

- `mmb4m-0.1.0-silicon.zip` for Apple Silicon, any Mac with an M-series chip
- `mmb4m-0.1.0-intel.zip` for an Intel Mac

Each holds the single file `mmbasic`.

macOS blocks a downloaded program without an Apple developer certificate, and this one has none yet. Once per download:

```sh
xattr -dr com.apple.quarantine mmbasic
./mmbasic
```

[Running MMB4M](docs/running.md) covers the `PATH` and the command line.

## Running programs

```sh
mmbasic prog.bas          # run and exit
mmbasic -i prog.bas       # run, then stay at the prompt
mmbasic                   # prompt only
```

`QUIT` exits. At the prompt, Ctrl-C stops the running program and gives the prompt back; in batch mode it ends the run with exit code 130.

## Colour Maximite 2 programs

Most run unchanged. The ones that do not are almost always graphics: `MODE` and `PAGE` do not exist here, and `PRINT` never draws to a surface.

Start the interpreter in Colour Maximite 2 mode and `MODE`, `PAGE` and the rest come back:

```sh
mmbasic -s "Colour Maximite 2" prog.bas
```

[Colour Maximite 2 programs](docs/cmm2.md) says what comes back and what does not.

MMB4M is not the CMM2 firmware. It is MMBasic for Linux 0.8, an alpha from its development branch, and its language follows MMBasic 6.0 for the PicoMite. The Colour Maximite 2 mode simulates the device name, `MODE`, `PAGE` and the controllers. That is enough to write and test a CMM2 program's logic at a desk; graphics, sound and timing are close, not identical.

## What is different

The language is MMBasic 6, and each machine's page carries its current user manual: [PicoMite](https://geoffg.net/picomite.html) and [PicoMite VGA](https://geoffg.net/picomitevga.html), the manual MMB4L follows and therefore the one this port follows, and [Colour Maximite 2](https://geoffg.net/maximite.html), whose firmware is a separate release of MMBasic with a manual of its own. This is what a Mac does differently.

| What | On a Mac | Details |
| --- | --- | --- |
| `MODE`, `PAGE` | There are no screen modes. A program opens its own window with `GRAPHICS WINDOW 0, 640, 480` and draws into it. Both commands do work when MMB4M is started with `-s "Colour Maximite 2"` | [Graphics](docs/graphics.md) |
| `FRAMEBUFFER` | Not implemented, and Colour Maximite 2 mode does not bring it back. `GRAPHICS BUFFER` with `GRAPHICS COPY` draws off-screen the same way | [What MMB4M cannot do](docs/limits.md) |
| `PRINT` | Writes to the terminal, never to a graphics surface, in Colour Maximite 2 mode as well. `TEXT x, y, s$` draws onto the surface `GRAPHICS WRITE` selected | [Graphics](docs/graphics.md) |
| `SETPIN`, `PIN`, `PULSE`, `FLASH` | A Mac has no microcontroller pins, so there is nothing to switch to | [What MMB4M cannot do](docs/limits.md) |
| Serial | Nothing above 230400 baud; macOS defines no higher rate, and setting one fails. Ports are named `/dev/cu.*`, which `ls /dev/cu.*` lists | [What MMB4M cannot do](docs/limits.md) |
| Timing | macOS is not a real-time system, so `PAUSE` and `SETTICK` drift more than on a microcontroller. `SETTICK` works, `SETTICK FAST` does not | [What MMB4M cannot do](docs/limits.md) |
| `EDIT` | Opens MMBasic's own full-screen editor in the terminal, the one a PicoMite has: F1 saves, F2 saves and runs, ESC leaves. `OPTION EDITOR VIM`, `VSCODE`, `NANO` or a command of your own switches to another editor, `OPTION EDITOR INTERNAL` switches back | [Editing programs](docs/editor.md) |
| `SAVE` | Not needed: the file on disk is the program, and F1 in the editor writes it. `SAVE "prog.bas"` answers `Unknown SAVE subcommand`, because `SAVE IMAGE` does exist | [Editing programs](docs/editor.md) |
| `GAMEPAD` | `DEVICE GAMEPAD OPEN` and `CLOSE` instead, for a controller connected to the Mac. The function `GAMEPAD()` is not there | [What exists here and not on a PicoMite](docs/commands.md) |
| `!cmd` | At the prompt, short for `SYSTEM`: `!ls` lists the directory. `!cd foo` is the exception and becomes `CHDIR "foo"`, because every other command runs in a forked process | [Running MMB4M](docs/running.md) |
| `CONSOLE` | Sets the colours, the cursor and the title of the terminal. `CONSOLE GETSIZE` reads its size, which a terminal can change and a PicoMite screen cannot | [What exists here and not on a PicoMite](docs/commands.md) |
| `OPTION CODEPAGE` | Decides how bytes 128 to 255 print. A terminal is Unicode and a Colour Maximite 2 screen is not, so `OPTION CODEPAGE CMM2` is needed before a CMM2 program's box characters come out right. It lasts for the session, not beyond | [Options](docs/options.md) |
| Command line | `mmbasic prog.bas` runs and exits, `-i` stays at the prompt afterwards, `-d` sets the starting directory, `-s` simulates another device, `-l` sets the log level, `-v` prints the version | [Running MMB4M](docs/running.md) |
| CMM2 programs | Start with `-s "Colour Maximite 2"`, or `-s CMM2` for short. Do not switch device from inside a running program: that faults at the next graphics command | [Colour Maximite 2 programs](docs/cmm2.md) |
| This version | 0.1.0, one known defect, and a list of what has been tried on a Mac and what has not | [Known problems](docs/known-issues.md) |
| Building it | One `make`, with the Xcode Command Line Tools and cmake. SDL2 is compiled in, so the result is a single file | [Building MMB4M](docs/building.md) |

## This is not an official release

MMB4M is not maintained by the authors of MMBasic and carries no promise from them. The supported versions are MMBasic for Linux and real Colour Maximite 2 hardware.

Geoff Graham gave permission on 2026-08-25 on two conditions: the name "MMBasic" stays, and the original copyright message keeps appearing at startup. Both are met.

## Credit and licence

MMBasic is the work of **Geoff Graham** and **Peter Mather**. [MMBasic for Linux](https://github.com/thwill1000/mmb4l), including its graphics, sound and console, is the work of **Thomas Hugo Williams**, and it is his interpreter you are running. MMB4M changes what is needed to build and run it on a Mac and nothing else.

My thanks to Thomas Hugo Williams for years of work on MMB4L, and for the permission to build on it. Without that work there would be no Mac version.

The [changelog](CHANGELOG.md) records which MMBasic for Linux commit each version was built from.

MMBasic's own modified BSD licence, five numbered conditions, reproduced word for word in [LICENSE.MMBasic](licenses/MMB4L/LICENSE.MMBasic). [LICENSE](LICENSE) says who holds what.

## First version

MMB4M 0.1.0 is the first version of this port, built and tested by one person on one Apple Silicon Mac. Not everything has been tried; [known problems](docs/known-issues.md) says what has and what has not. If something does not work, open an [issue](https://github.com/TomSchimana/mmb4m/issues) or get in touch through [schimana.net](https://schimana.net). Questions about MMBasic itself belong on [The Back Shed](https://www.thebackshed.com/forum/ViewForum.php?FID=16).
