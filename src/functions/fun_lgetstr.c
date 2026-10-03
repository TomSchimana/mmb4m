/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_lgetstr.c

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
#include "../common/streamio.h"

void fun_lgetstr(void) {
    void *ptr1 = NULL;
    char *p;
    char *s = NULL;
    int64_t *src = NULL;
    int start, nbr, j;
    getargs(&ep, 5, DELIM_COMMA);
    if (argc != 5) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        src = (int64_t *)ptr1;
        s = (char *)&src[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    j = (vartbl[VarIndex].dims[0] - mmb_options.base) * 8;
    start = getint(argv[2], 1, j);
    nbr = getinteger(argv[4]);
    if (nbr < 1 || nbr > MAXSTRLEN) ERROR_NUMBER_OUT_OF_BOUNDS;
    if (start + nbr > src[0]) nbr = src[0] - start + 1;
    if (nbr < 0) nbr = 0;  // start is beyond the end of the long string: return an empty string
    sret = GetTempStrMemory();  // this will last for the life of the command
    s += (start - 1);
    p = sret + 1;
    *sret = nbr;
    while (nbr--) *p++ = *s++;
    *p = 0;
    targ = T_STR;
}

/**
 * LINPUT(array%(), [#]fnbr, nbr): reads up to nbr bytes from file fnbr into
 * the long string, which then holds exactly what was read; returns that
 * count. As on the PicoMite, files only, not serial ports or the console.
 */
void fun_linput(void) {
    getargs(&ep, 5, DELIM_COMMA);
    if (argc != 5) ERROR_ARGUMENT_COUNT;
    void *ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (!(vartbl[VarIndex].type & T_INT)) ERROR_ARG_NOT_INTEGER_ARRAY(1);
    if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
    if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
    if (vartbl[VarIndex].type & T_CONST) ERROR_CANNOT_CHANGE_A_CONSTANT;
    int64_t *dest = (int64_t *) ptr1;
    const int capacity = (vartbl[VarIndex].dims[0] - mmb_options.base) * 8;
    const int fnbr = parse_file_number(argv[2], true);
    if (fnbr == -1) ON_FAILURE_ERROR(kFileInvalidFileNumber);
    const int nbr = getint(argv[4], 0, capacity);
    if (fnbr == 0 || !streamio_is_file(fnbr)) error_throw_ex(kError, "Input from file only");
    dest[0] = (int64_t) streamio_read(fnbr, (char *) &dest[1], nbr);
    g_integer_rtn = dest[0];
    g_rtn_type = T_INT;
}

