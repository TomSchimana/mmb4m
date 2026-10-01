/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_load.c

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/graphics.h"
#include "../common/image.h"
#include "../common/parse.h"
#include "../common/program.h"
#include "../common/utility.h"
#include "../core/commandtbl.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

/**
 * LOAD BMP file$ [, x] [, y]
 * LOAD IMAGE file$ [, x] [, y]
 */
static MmResult cmd_load_bmp(const char *p) {
    getargs(&p, 5, DELIM_COMMA);
    if (argc < 1 || argc > 5 || (argc % 2 == 0)) return kArgumentCount;

    char *filename = GetTempStrMemory();
    ON_FAILURE_RETURN(parse_filename(argv[0], filename, STRINGSIZE));

    const int x = has_arg(2) ? getinteger(argv[2]) : 0;
    const int y = has_arg(4) ? getinteger(argv[4]) : 0;

    return image_load_bmp(graphics_current, filename, x, y);
}

/** LOAD DATA file$, address: the file's bytes written to memory from address on, as on the PicoMite. */
static MmResult cmd_load_data(const char *p) {
    getargs(&p, 3, DELIM_COMMA);
    if (argc != 3) return kArgumentCount;
    char *filename = GetTempStrMemory();
    ON_FAILURE_RETURN(parse_filename(argv[0], filename, STRINGSIZE));
    char *to = (char *) get_poke_addr(argv[2]);
    FILE *f = fopen(filename, "rb");
    if (!f) return errno;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        memcpy(to, buf, n);
        to += n;
    }
    fclose(f);
    return kOk;
}

/** LOAD FONT file$ */
static MmResult cmd_load_font(const char *p) {
    ERROR_UNIMPLEMENTED("LOAD FONT");
    return kUnimplemented;
}

/** LOAD GIF file$ [, x] [, y] */
static MmResult cmd_load_gif(const char *p) {
    ERROR_UNIMPLEMENTED("LOAD GIF");
    return kUnimplemented;
}

/** LOAD JPG file$ [, x] [, y] [, mode] [, ximage] [, yimage] */
static MmResult cmd_load_jpg(const char *p) {
    if (!graphics_current) error_throw(kGraphicsInvalidWriteSurface);

	getargs(&p, 13, DELIM_COMMA);
    if (argc < 1 || argc > 13 || (argc % 2 == 0)) return kArgumentCount;

    char *filename = GetTempStrMemory();
    ON_FAILURE_RETURN(parse_filename(argv[0], filename, STRINGSIZE));

    const int x = has_arg(2) ? getinteger(argv[2]) : 0;
    const int y = has_arg(4) ? getinteger(argv[4]) : 0;
    const int mode = has_arg(6) ? getinteger(argv[6]) : -1;
    if (mode < -1 || mode > 7) {
        return mmresult_ex(kInvalidArgument, "Invalid mode: %d; valid modes are -1 to 7", mode);
    }
    const int ximage = has_arg(8) ? getinteger(argv[8]) : 0;
    const int yimage = has_arg(10) ? getinteger(argv[10]) : 0;
    const int scale = has_arg(12) ? getinteger(argv[12]) : 1;

    return image_load_jpg(graphics_current, filename, x, y, (ImageDitherMode) mode, ximage, yimage,
                          scale);
}

/** LOAD PNG file$ [, x] [, y] [, transparency_cut_off] */
static MmResult cmd_load_png(const char *p) {
    if (!graphics_current) error_throw(kGraphicsInvalidWriteSurface);

	getargs(&p, 7, DELIM_COMMA);
    if (argc == 0 || argc > 7) return kArgumentCount;

    char *filename = GetTempStrMemory();
    ON_FAILURE_RETURN(parse_filename(argv[0], filename, STRINGSIZE));

    const int x = has_arg(2) ? getinteger(argv[2]) : 0;
    const int y = has_arg(4) ? getinteger(argv[4]) : 0;
    int transparent = has_arg(6) ? getint(argv[6], 0, 15) : 0;
    int force = 0;
    if (transparent > 4) {
        force = transparent << 4;
        transparent = 4;
    }

    return image_load_png(graphics_current, filename, x, y, transparent, force);
}

/**
 * LOAD file$ [,C] [,R], as on the PicoMite. R runs the program at once and is
 * the only form allowed in a program, which chains to the other one. C, which
 * removes comments, blank lines and spaces, is what the loader always does here.
 */
static MmResult cmd_load_default(const char *p) {
    getargs(&p, 5, DELIM_COMMA);
    if (argc != 1 && argc != 3 && argc != 5) return kArgumentCount;
    bool run = false;
    for (int i = 2; i < argc; i += 2) {
        if (checkstring(argv[i], "R")) {
            run = true;
        } else if (!checkstring(argv[i], "C")) {
            return kSyntax;
        }
    }
    if (!run && CurrentLinePtr) return mmresult_ex(kError, "Invalid in a program");

    char *filename = GetTempStrMemory();
    ON_FAILURE_RETURN(parse_filename(argv[0], filename, STRINGSIZE));
    if (!run) return program_load_file(filename);

    // As RUN file$ would.
    static char run_args[STRINGSIZE + 2];
    snprintf(run_args, sizeof(run_args), "\"%s\"", filename);
    cmdline = run_args;
    cmd_run();
    return kOk;
}

void cmd_load(void) {
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "BMP"))) {
        result = cmd_load_bmp(p);
    } else if ((p = checkstring(cmdline, "DATA"))) {
        result = cmd_load_data(p);
    } else if ((p = checkstring(cmdline, "IMAGE"))) {
        result = cmd_load_bmp(p);
    } else if ((p = checkstring(cmdline, "JPG"))) {
        result = cmd_load_jpg(p);
    } else if ((p = checkstring(cmdline, "GIF"))) {
        result = cmd_load_gif(p);
    } else if ((p = checkstring(cmdline, "FONT"))) {
        result = cmd_load_font(p);
    } else if ((p = checkstring(cmdline, "PNG"))) {
        result = cmd_load_png(p);
    } else {
        result = cmd_load_default(cmdline);
    }
    ON_FAILURE_ERROR(result);
}
