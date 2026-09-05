[← MMB4M](../README.md)

# Graphics

There are no screen modes. A program opens a window and draws into it, and that window is a real macOS window, movable and closable.

```basic
GRAPHICS WINDOW 0, 640, 480, , , "My program"
GRAPHICS WRITE 0
CLS
CIRCLE 320, 240, 100
```

Drawing commands go to the surface `GRAPHICS WRITE` selected. Until one is selected they stop the program with `Invalid graphics write surface`.

## Three kinds of surface

A **window** is what you see. `GRAPHICS WINDOW id, width, height`, with ids from 0 to 255.

A **buffer** is off-screen. `GRAPHICS BUFFER id, width, height`. Drawing a frame into a buffer and copying it to the window when it is complete avoids flicker.

A **sprite** is a small image with transparency, made with `GRAPHICS SPRITE` or `SPRITE READ` and moved with the `SPRITE` commands.

Every drawing command works on all three, and `BLIT` moves pixels between any of them.

```basic
GRAPHICS BUFFER 1, 640, 480
GRAPHICS WRITE 1
' draw the frame
GRAPHICS COPY 1 TO 0
```

`GRAPHICS COPY src TO dst [, when] [, transparent]`. Pass `1` as the last argument, `GRAPHICS COPY 1 TO 0, , 1`, and black pixels on the source are left out. Surfaces of different sizes can be copied to each other. The source lands at the top left, clipped if it is bigger, leaving the rest untouched if it is smaller.

## Placing and scaling a window

```basic
GRAPHICS WINDOW id, width, height [, x] [, y] [, title$] [, scale] [, interrupt]
```

Leave `x` and `y` out, or pass -1, and the window is centred. `scale` multiplies the pixel size, which is how a 320x240 program fills a modern display. A window that would not fit is scaled down automatically, in whole steps above 2 and in steps of 0.05 below.

## Reacting to the window

```basic
GRAPHICS WINDOW 0, 640, 480, , , "Demo", 2, my_events

SUB my_events(window_id%, event_id%)
  IF event_id% = WINDOW_EVENT_CLOSE THEN QUIT
END SUB
```

The constants are defined for you: `WINDOW_EVENT_CLOSE`, `WINDOW_EVENT_FOCUS_GAINED`, `WINDOW_EVENT_FOCUS_LOST`, `WINDOW_EVENT_MINIMISED`, `WINDOW_EVENT_MAXIMISED`, `WINDOW_EVENT_RESTORED`. Without a handler, closing the window ends the program.

`GRAPHICS LIST` shows what exists, `GRAPHICS DESTROY id` or `GRAPHICS DESTROY ALL` clears up.

## PRINT does not draw

`PRINT` writes to the terminal, always, even with a window open and selected, and even while simulating a Colour Maximite 2. `TEXT` puts text on a surface.

On a Colour Maximite 2 the two are the same thing.

## HRES and VRES

With no surface selected they describe the terminal in pixels of a nominal 8x12 character cell: an 80 by 40 terminal reports 640 by 480. `MM.INFO(HRES C)` and `MM.INFO(VRES C)` give characters. With a surface selected they describe the surface.

## MODE and PAGE

Both come back when you start the interpreter as a Colour Maximite 2. See [Colour Maximite 2 programs](cmm2.md).
