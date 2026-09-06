[← MMB4M](../README.md)

# Editing programs

```basic
EDIT "myprogram.bas"
EDIT
```

`EDIT` opens MMBasic's own full-screen editor, the one both devices have, in the terminal. Without a filename it edits the current program file, the one `LOAD` or `RUN` set.

`LIST`, `EDIT` and `RUN` read the file from disk first, so what you see and what runs is always what is in the file. A change made in another window is picked up by the next command, and MMBasic never holds a copy that differs from the file.

## Not the same as on a PicoMite

A PicoMite holds the program itself in flash memory, and there `EDIT` and `EDIT "prog.bas"` work on two different things: the copy in flash, and a file on the SD card. F2 runs the program only in the first case.

MMB4M works the way a Colour Maximite 2 does. There is no second copy anywhere: "the current program" is a filename, set by `LOAD`, `RUN` or `EDIT "prog.bas"`, and the editor always edits that file.

- `EDIT` without a filename and without a current program answers `Error: Nothing to edit`, and the editor does not open. A CMM2 says exactly the same.
- F2 saves and runs, whether the editor was opened with a filename or without.

So a program written for a PicoMite that relies on the flash copy behaves differently here. One written on a CMM2 does not.

The key list stands at the bottom of the screen.

```
ESC        leave the editor, keeping the file as it was
F1         save and leave the editor
F2         save and run the program
F3         find, Shift+F3 finds again
F4         start marking; then DEL deletes, F4 cuts, F5 copies
F5         paste
```

There is no `SAVE` command for programs: you edit the real file, and F1 writes it.

## Using your own editor instead

`OPTION EDITOR` exists only here. A Colour Maximite 2 or a PicoMite has no choice of editor.

```basic
OPTION EDITOR VIM
OPTION EDITOR "nvim +${line} ${file}"
OPTION EDITOR VSCODE
OPTION EDITOR NANO
OPTION EDITOR INTERNAL
```

The names that matter on a Mac are `INTERNAL`, `VIM`, `NANO`, `VSCODE` and `SUBLIME`; `ATOM`, `CODE`, `GEANY`, `GEDIT`, `LEAFPAD`, `VI` and `XED` are known too, most of them Linux editors. `DEFAULT` means `INTERNAL`. An editor that is not in the list is given as a string, which is run as a shell command: `${file}` becomes the filename, already quoted, and `${line}` the line to put the cursor on.

- **vim** ships with macOS: `OPTION EDITOR VIM`.
- **nvim** from Homebrew, `brew install neovim`, then the string above.
- **VS Code** is reached through its `code` command, which the app does not install by itself: "Shell Command: Install 'code' command in PATH" in VS Code, then `OPTION EDITOR VSCODE`.
- **nano**: see below; the one that ships with macOS is not GNU nano.

MMBasic starts the editor through `sh` with the `PATH` of the terminal you started it from, and never reads `.zshrc`, so shell aliases do not apply.

The setting is kept: it goes into `~/.mmbasic/mmbasic.options` the moment you set it and holds for every later start. `OPTION RESET EDITOR` puts the internal editor back. `Error: Editor could not be run` means the kept setting names a program the shell cannot find.

With an external editor, MMBasic takes the file back when you close the editor. No key runs the program from there. If you rename the file while saving, MMBasic does not notice and keeps the old name as the current program file.

## nano with the MMBasic keys

The `nano` that ships with macOS is pico: `/usr/bin/nano` is a symlink to `pico` and reads no configuration. `OPTION EDITOR NANO` on a stock Mac opens pico. For GNU nano:

```sh
brew install nano
```

`nano --version` shows which one is on the `PATH`: GNU nano prints a version, pico opens an empty buffer.

The two files under `resources/` give GNU nano 4.8 and later the key layout of the MMBasic editor and syntax colouring for BASIC:

```sh
mkdir -p ~/.mmbasic
cp resources/mmbasic.nanorc ~/.mmbasic/
cp resources/mmbasic.syntax.nanorc ~/.mmbasic/
```

```
Ctrl+A              help
Ctrl+Q or F1        leave the editor
Ctrl+S or F2        save
Ctrl+O or F7        insert another file here

Ctrl+B              first line
Ctrl+E              last line
Ctrl+J              go to line
Ctrl+P / Ctrl+N     previous / next word
Ctrl+L              where the cursor is

Ctrl+K              delete the line or the selection
Ctrl+C / Ctrl+X     copy / cut
Ctrl+V              paste
Ctrl+Space or F4    start or end a selection

Ctrl+F              find
Ctrl+G or F3        find next
Ctrl+H              replace
Ctrl+Z / Ctrl+Y     undo / redo

Alt+A               show or hide the key list at the bottom
Alt+L               show the cursor position permanently
```

Everything else stays nano's own. Tab indents, Shift+Tab unindents, Alt+3 comments a selection.

## When the error is in an include file

`EDIT` after an error opens the file the error came from, which may be a `.INC` rather than your `.BAS`. `EDIT CURRENT` opens the main program.
