# Changelog

## 0.2.0, 2026-10-01

MMB4M 0.2 adds many commands from the PicoMite V6.04 and raises most of its limits to those of MMBasic for Windows. Programs now have 128 MB for variables, arrays and strings instead of 1 MB, and an array can have more than 32767 elements per dimension. Memory is managed differently, so string handling and SUB calls stay fast when a program holds large arrays. New commands include `BIT()` and `BYTE()`, `DO UNTIL`, `TRIM$`, commands that work on whole arrays, AES encryption, base64, `SAVE DATA` and `LOAD DATA`, and PID controllers. `MID$` as a statement, `TIMER`, `END` and `BOUND` now work as on the PicoMite and so differently from 0.1. Calling a `CSUB` stops with a clear error message.

- `BIT(x%, n)` reads and sets single bits of an integer, `BYTE(s$, n)` single bytes of a string, and `FLAG(n)`, `FLAGS` and `MM.FLAGS` hold 64 flags for the whole program. `PEEK(BP a%)`, `PEEK(SP a%)` and `PEEK(WP a%)` read a byte, short or word and move `a%` past it. From PicoMite V6.04.00RC2, commit c655589
- Arrays take bounds up to 2147483647 per dimension instead of 32767, as on the PicoMite RP2350, the Colour Maximite 2 and MMBasic for Windows. Each variable's entry in the variable table grows from 64 to 80 bytes, so offsets read with `PEEK(VARTBL)` and `PEEK(VARHEADER)` move
- The capacities of MMBasic for Windows: 128 MB for variables, arrays and strings instead of 1 MB, 1 MB of program instead of 512 KB, 2048 variables, 128 nested `FOR` and `DO` loops each, 128 open files, and 10 sprite layers. `FILES` lists 2048 entries, as on the Colour Maximite 2
- String operations and `SUB` calls no longer slow down beside large arrays: temporary memory is taken from the bottom of the heap, as on the PicoMite. 20000 `STR$()` beside a 96 MB array take 13 ms instead of 10 s. A string parameter or a `LOCAL` variable still costs time there. From PicoMite V6.04.00RC2, commit ee680d4
- A program takes 256 `#DEFINE` lines, as the limit says; it used to stop at 126 with `Out of temporary memory buffers`. Found in MMB4M 0.1.3; the defect is in MMB4L v0.8-alpha.1, commit 8e98d84
- `MID$(a$, n, m) = b$` replaces `m` characters with all of `b$` and changes the length of `a$`, as on the PicoMite and the Colour Maximite 2; before, only `m` characters of `b$` went in. A length of 0 inserts. The error for a length beyond the string now names 0 as its lower end. From PicoMite V6.04.00RC2, commit c655589
- `TIMER` is a float in milliseconds with microseconds, as on the PicoMite, the Colour Maximite 2 and MMBasic for Windows; it used to be whole milliseconds. From PicoMite V6.04.00RC2, commit c655589
- `END cmd$` runs `cmd$` as though typed at the prompt once the program has ended, a `SUB MM.END` runs whenever the program ends, and `END NOEND` skips it. `END exitcode` still sets the exit code. From PicoMite V6.04.00RC2, commit c655589
- `BOUND` on a dimension the array does not have is an error, `Dimensions`, and on a variable that is not an array `Expected an array`; it used to return 0, which is also a real upper bound under `OPTION BASE 0`. From PicoMite V6.04.00RC2, commit c655589
- `DO UNTIL condition ... LOOP` tests at the top of the loop; it used to raise `DO has an UNTIL test`. From PicoMite V6.04.00RC2, commit c655589
- `MM.ERRLINE` gives the line of the last error, 0 if there is none. From PicoMite V6.04.00RC2, commit c655589
- `TRIM$(s$ [, mask$] [, L|R|B])` removes characters from either end of a string, and `BASE$(base, n [, digits])` writes a number in any base from 2 to 36. From PicoMite V6.04.00RC2, commit c655589; `BASE$` is also on the Colour Maximite 2 and MMBasic for Windows
- `OPTION ESCAPE` makes string constants and quoted `DATA` take escape sequences such as `\n`, `\t`, `\q` for a quote, `\065` and `\&41`, and `MM.ESC` says whether it is set. From PicoMite V6.04.00RC2, commit c655589; `MM.ESC` from the Colour Maximite 2 V6
- `LMID(ls%(), start [, num]) = s$` replaces part of a long string, changing its length, and `LINPUT(ls%(), fnbr, nbr)` reads up to `nbr` bytes of a file into one. From PicoMite V6.04.00RC2, commit c655589
- `MM.INFO` knows `UPTIME`, `VARCNT`, `MAX VARS`, `HEAP` (free bytes), `FREE SPACE`, `DISK SIZE`, `MODIFIED file$`, `SOUND`, `TRACK`, `MODE`, `FAST TIME` and `FONTCOUNT`. From PicoMite V6.04.00RC2, commit c655589; `MODE` as on the Colour Maximite 2, `FAST TIME` and `FONTCOUNT` as in MMBasic for Windows, `FAST TIME` counting nanoseconds here
- `OPTION PROFILING ON` counts how often each command, `SUB` and `FUNCTION` runs and how long the routines take, and `END` prints the report; `LIST PROFILE` then lists each program line with its count and average time. The report from PicoMite V6.04.00RC2, commit c655589; `LIST PROFILE` as on the Colour Maximite 2
- `LOAD file$, R` loads and runs a program, also from inside one, which chains to it; `,C` is accepted and is what loading always does here. `SAVE file$` writes a copy of the program's source, `SAVE DATA` and `LOAD DATA` move bytes between memory and a file, `MEMORY PRINT` and `MEMORY INPUT` between memory and an open file, and `FLUSH #n` writes out a file's buffer. From PicoMite V6.04.00RC2, commit c655589
- A program with `CFUNCTION` blocks, as the Colour Maximite 2 V6 writes them, loads and skips them as it does `CSUB` blocks, instead of stopping with `Unknown command`. `>>>` shifts right keeping the sign, as on the Colour Maximite 2, where `>>` does not, `MM.POS` gives the cursor's column, and a `SUB MM.PROMPT` prints the prompt instead of `> `. From the Colour Maximite 2 5.07 and V6
- Calling a `CSUB` or `CFUNCTION` stops with `Unsupported on current device/platform` instead of `Invalid character: 48`; before, its block of machine code was run as lines of BASIC. Found in MMB4M 0.1.4
- `OPTION MILLISECONDS ON` makes `TIME$` give `HH:MM:SS.mmm`, and `OPTION CONTINUATION LINES ON` makes `LIST` split long lines with ` _` and loading a program join such lines again; it is saved like the other options. From PicoMite V6.04.00RC2, commit c655589
- `SYNC time% [, U|M|S]` sets a repeating clock and `SYNC` waits for its next tick. From PicoMite V6.04.00RC2, commit c655589
- `AUTOSAVE N "file.bas"` takes a program without echoing it. From the Colour Maximite 2 V6
- `MATH CLAMP in(), lo, hi, out()` limits every element, `MATH V_PRINT a%(), HEX` prints in hex, and `MATH SENSORFUSION MADGWICK|MAHONY` fuses accelerometer, gyroscope and magnetometer readings. From PicoMite V6.04.00RC2, commit c655589
- `MATH SINC` smooths or resamples x/y data with a windowed sinc filter, and `MATH(UPSAMPLE in(), out(), infreq, outfreq)` raises the sample rate of a signal. `SINC` from PicoMite V6.04.00RC2, commit c655589; `UPSAMPLE` as in MMBasic for Windows
- `MATH(BASE64 ENCODE|DECODE in, out)` for strings and byte arrays, and `LONGSTRING BASE64 ENCODE|DECODE` for long strings. From PicoMite V6.04.00RC2, commit c655589
- `MATH AES128 ENCRYPT|DECRYPT CBC|ECB|CTR key, in, out [, iv]` encrypts and decrypts strings and byte arrays with AES-128, and `LONGSTRING AES128` does the same for long strings. CBC and CTR put a random initialisation vector in front of the output unless one is given. From PicoMite V6.04.00RC2, commit ee680d4
- `MATH PID INIT`, `START` and `STOP` run up to eight PID controllers in the background, each calling its `SUB` as an interrupt every sample time, and `MATH(PID channel, setpoint, measurement)` updates one. From PicoMite V6.04.00RC2, commit c655589
- `MEMORY PACK` and `MEMORY UNPACK` pack integers into values of 1, 4, 8, 16 or 32 bits and back. From PicoMite V6.04.00RC2, commit c655589
- `ARRAY SET`, `ARRAY ADD`, `ARRAY SLICE` and `ARRAY INSERT` work on number and string arrays; `SLICE` and `INSERT` convert between integer and float. From PicoMite V6.04.00RC2, commit c655589
- `b%() = a%()` copies a whole array into another of the same type and number of elements. From PicoMite V6.04.00RC2, commit c655589
- `mmbasic -h` names every device `-s` can simulate, each by its short form

## 0.1.4, 2026-09-30

Fixes twelve defects: two hangs and crashes, wrong results at the edges of the number range, and names that silently read as zero.

- A program with all ten file numbers open no longer hangs when it ends. Found in MMB4M 0.1.3; the defect is in MMB4L v0.8-alpha.1, commit 8e98d84
- A program just under the 512 KB program memory says `Program too long` instead of crashing or reporting an internal fault. Found in MMB4M 0.1.3; the defect is in MMB4L v0.8-alpha.1, commit 8e98d84
- `MATH M_INVERSE` can be repeated without running out of memory, and inverts a 1x1 array. From PicoMite V6.04.00, commit c994e92
- `MAX(-1E300, -2E300)` is `-1e+300`, and `MIN` likewise; both used to stop at ±3.4e+38. From PicoMite V6.04.00, commit 50c9aa0
- `ABS` accepts floats of 2^63 and more instead of raising `Number too large`. From PicoMite V6.04.00, commit 16d0413
- Infinity and NaN print as `INF`, `-INF` and `NAN` instead of `0e+2147483647`. From PicoMite V6.04.00, commit a40112b
- `INT(1E30)`, `FIX(-1E30)`, NaN and 2^63 raise `Number too large` when turned into an integer instead of giving a wrong number. From PicoMite V6.04.00, commit a40112b; the check on 32-bit values found in MMB4M 0.1.3, a defect in MMB4L v0.8-alpha.1, commit 8e98d84
- `RANDOMIZE` without an argument seeds from the clock instead of being a syntax error. From PicoMite V6.04.00, commit 1457087
- `STATIC` belongs to the SUB it is written in; before, a call to another SUB just above it made the two share the variable, and an interrupt SUB got a second one. From PicoMite V6.04.00, commit b0f448e
- `STATIC` in a routine reached by `GOSUB` outside any SUB raises `Invalid here`, as on the PicoMite. From PicoMite V6.04.00, commit b0f448e
- `MM.FONTHEIGHT`, `MM.FONTWIDTH`, `MM.HPOS`, `MM.VPOS`, `MM.WIDTH` and `MM.HEIGHT` exist, as on the PicoMite and the Colour Maximite 2; they used to be variables worth 0. Names from PicoMite V6.04.00, values as `MM.INFO()` gives them
- `MATH CRC8`, `CRC12`, `CRC16` and `CRC32` honour their reverse-output argument. From PicoMite V6.03.02b2
- `MATH CHI` and `CHI_P` refuse more than 50 degrees of freedom instead of reading past their table, and count them right under `OPTION BASE 1`. From PicoMite V6.03.02b2

## 0.1.3, 2026-09-28

Fixes PLAY TONE, and withdraws two commands that were never finished. Now built from MMBasic for Linux 0.8 alpha 1, branch `main`, commit `8e98d84`.

- `PLAY TONE` plays the right channel, at its own frequency. Before, `PLAY TONE 440, 660` sounded 440 Hz on the left and nothing on the right
- `MM.INFO(CURRENT FUNCTION)` and `LIST CALLS` are gone. Both were unfinished in 0.1.0 to 0.1.2, and upstream withdrew them for this release
- fonts 8 and 9 from the PicoMite
- sound is handed to macOS in blocks of 1024 samples instead of one at a time, upstream's fix against crackling

## 0.1.2, 2026-09-06

Fixes four defects: two crashes, one command that wrote a file it should not, and one refusal that said nothing useful.

- `OPTION SIMULATE` inside a running program no longer faults at the next graphics command
- `MM.INFO$(SDCARD)`, `MM.INFO(CPUSPEED)` and `MM.INFO$(DRIVE)` no longer end the process with a segmentation fault
- `EDIT` after an error at the prompt no longer creates and edits a file called `<PROMPT>`
- `MODE` and `PAGE` name the command line that brings them back: `mmbasic -s CMM2` or `mmbasic -s PicoMiteVGA`

## 0.1.1, 2026-09-05

Fixes reading from the console. In 0.1.0 both commands printed their prompt and then stopped before reading a character.

- `INPUT` and `LINE INPUT` read the console again, instead of failing with `Invalid file number`

## 0.1.0, 2026-09-05

First version, for Apple Silicon and Intel, one zip per architecture holding the single file `mmbasic`. Built from MMBasic for Linux, branch `develop-v0.8-8`, commit `529fded`.

- builds and runs on macOS, Apple Silicon and Intel
- `PATH_MAX` comes from the macOS header, so paths stop at 1024 characters
- serial baud rates above 230400 are gone; macOS does not have them
- `EDIT` opens MMBasic's own editor by default; `OPTION EDITOR` still selects nano, VS Code or any other
- the port's copyright line in the startup banner, under the original three
- the banner, `--version` and `MM.INFO(VERSION)` report the port's own version and `macOS arm64` or `macOS x86_64`
