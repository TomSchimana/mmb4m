/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_mid.c

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

#include <string.h>

#include "../common/mmb4l.h"
#include "../core/tokentbl.h"

#define ERROR_NOT_A_STRING              error_throw_ex(kError, "Expected a string")
#define ERROR_SELECTION_EXCEEDS_LENGTH  error_throw_ex(kError, "Selection exceeds length of string")

void cmd_mid(void) {
    // Left hand side of expression.
    getargs(&cmdline, 5, DELIM_COMMA);
    findvar(argv[0], V_NOFIND_ERR);
    if (vartbl[VarIndex].type & T_CONST) ERROR_CANNOT_CHANGE_A_CONSTANT;
    if (!(vartbl[VarIndex].type & T_STR)) ERROR_NOT_A_STRING;
    const int size = vartbl[VarIndex].size;
    char *sourcestring = getstring(argv[0]);
    const int start = getint(argv[2], 1, sourcestring[0]);
    int num = (argc == 5) ? getint(argv[4], 0, sourcestring[0]) : -1;
    if (start + (num < 0 ? 0 : num - 1) > sourcestring[0]) ERROR_SELECTION_EXCEEDS_LENGTH;

    // Find and consume '=' token.
    while (*cmdline && tokentbl_read(&cmdline) != tokenEQUAL) { }

    // Right hand side of expression.
    skipspace(cmdline);
    char *value = getstring(cmdline);
    char *p = &value[1];
    if (num == -1) {
        // Without a length as many characters are replaced as the string
        // holds from start on; its length stays.
        num = value[0];
        if (start + num - 1 > sourcestring[0]) num = sourcestring[0] - start + 1;
        memcpy(&sourcestring[start], p, num);
    } else if (num == value[0]) {
        memcpy(&sourcestring[start], p, num);
    } else {
        // As on the PicoMite: the selection is replaced by all of value,
        // and the string grows or shrinks by the difference.
        const int change = value[0] - num;
        if (sourcestring[0] + change > size) ERROR_STRING_TOO_LONG;
        memmove(&sourcestring[start + value[0]], &sourcestring[start + num],
                sourcestring[0] - (start + num - 1));
        sourcestring[0] += change;
        memcpy(&sourcestring[start], p, value[0]);
    }
}

/**
 * LMID(array%(), start [, num]) = s$: as MID$() = on a long string, as on the
 * PicoMite. Unlike its code, only the characters after the selection move,
 * so nothing is written past the long string's capacity.
 */
void cmd_lmid(void) {
    getargs(&cmdline, 5, DELIM_COMMA);
    if (argc != 3 && argc != 5) ERROR_ARGUMENT_COUNT;
    void *ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (!(vartbl[VarIndex].type & T_INT)) ERROR_ARG_NOT_INTEGER_ARRAY(1);
    if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
    if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
    if (vartbl[VarIndex].type & T_CONST) ERROR_CANNOT_CHANGE_A_CONSTANT;
    int64_t *dest = (int64_t *) ptr1;
    char *ls = (char *) &dest[1];
    const int capacity = (vartbl[VarIndex].dims[0] - mmb_options.base) * 8;
    const int length = (int) dest[0];
    const int start = getint(argv[2], 1, length) - 1;  // 0-based from here
    int num = (argc == 5) ? getint(argv[4], 0, length) : -1;
    if (num > 0 && start + num > length) ERROR_SELECTION_EXCEEDS_LENGTH;

    // Find and consume '=' token.
    while (*cmdline && tokentbl_read(&cmdline) != tokenEQUAL) { }
    skipspace(cmdline);
    if (!*cmdline) ERROR_SYNTAX;
    char *value = getstring(cmdline);
    const int vlen = (unsigned char) value[0];

    if (num == -1) {
        num = vlen;
        if (start + num > length) num = length - start;
        memcpy(&ls[start], &value[1], num);
    } else if (num == vlen) {
        memcpy(&ls[start], &value[1], num);
    } else {
        const int change = vlen - num;
        if (length + change > capacity) ERROR_STRING_TOO_LONG;
        memmove(&ls[start + vlen], &ls[start + num], length - (start + num));
        dest[0] += change;
        memcpy(&ls[start], &value[1], vlen);
    }
}

