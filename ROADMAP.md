# Roadmap

This roadmap brings MMB4M up to date with the current MMBasic developments by Peter Mather.

It is preliminary and can change as the work goes on.

## 0.1.4: fixes

- [ ] Calculations at the edges of the number range give wrong results, and infinity prints as garbage.
- [ ] Repeated matrix calculations run out of memory.
- [ ] A static variable in a subroutine can be shared by mistake with another subroutine.
- [ ] Some built-in system values, such as the font height, read as zero.
- [ ] Opening a tenth file at the same time hangs.
- [ ] A program close to the memory limit can crash instead of reporting that it is too long.

## 0.2: the language catches up

Programs written for a current PicoMite use language features MMB4M does not have yet. This release adds them.

- [ ] `BIT()` and `BYTE()`, to read and set single bits of a number and single bytes of a string.
- [ ] Arrays with more than 32767 elements per dimension, and more memory for them.
- [ ] More variables, deeper nesting of loops, more open files at once.
- [ ] Loops that test their condition at the start, trimming strings, the line number of the last error.
- [ ] Working on whole arrays in one statement.
- [ ] Encryption and base64.
- [ ] Saving and loading data, and more information about the running program.

## 0.3: mouse, images and controls

Programs can be operated with the mouse.

- [ ] The mouse in the graphics window.
- [ ] Loading images straight into sprites, rotating and resizing images, flood fill and curves.
- [ ] Buttons, sliders, lists, text boxes and the other GUI controls.

## 0.4: text screens, turtle graphics and sound

- [ ] Text screens with windows and panels, in the terminal and in the graphics window.
- [ ] Turtle graphics.
- [ ] The BBC Micro's sound commands, and playing samples.

## 0.5: 3D graphics and libraries

- [ ] The 3D engine, for programs that draw and move solid objects.
- [ ] Libraries: a program loads its own library of subroutines before it starts, as Peter Mather's recent games do.

## 0.6: PicoMite and Colour Maximite 2 programs

Programs written for these machines run unchanged.

- [ ] Tile maps and the raycaster.
- [ ] The PicoMite's flash memory slots.
- [ ] Colour palettes.
- [ ] The Colour Maximite 2's pages, its transparent colour and its framebuffer larger than the screen.

## 0.7: larger language features

- [ ] User-defined types.
- [ ] Starting another program with the variables kept.
- [ ] Regular expressions in string searches.
- [ ] Arrays with a single element, and functions that return arrays.

## 0.8: tools and the Mac

- [ ] The editor's newer features, online help and a file manager.
- [ ] Network connections and serial ports.

## Not planned

Commands for hardware a Mac does not have, such as pins, buses, sensors, displays, cameras and the board itself. Machine code routines written for the PicoMite's processor do not run on a Mac.
