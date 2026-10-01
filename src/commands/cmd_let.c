/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_dim.c

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
#include "../core/tokentbl.h"

// the LET command
// because the LET is implied (ie, line does not have a recognisable command)
// it ends up as the place where mistyped commands are discovered.  This is why
// the error message is "Unknown command"
/** Is p to end, less trailing spaces, a name with empty brackets, as in a%() ? */
static bool let_is_whole_array(const char *p, const char *end) {
    while (end > p && end[-1] == ' ') end--;
    return end - p >= 3 && end[-1] == ')' && end[-2] == '(';
}

/** The start and bytes of the whole array named at p, a%(), after checking it is one. */
static char *let_whole_array(const char *p, int *type, int *size, int64_t *bytes) {
    char *base = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if (vartbl[VarIndex].dims[0] <= 0 || base != vartbl[VarIndex].val.s) ERROR_SYNTAX;
    *type = vartbl[VarIndex].type & (T_INT | T_NBR | T_STR);
    *size = vartbl[VarIndex].size;
    int64_t n = 1;
    for (int i = 0; i < MAXDIM && vartbl[VarIndex].dims[i] != 0; i++) {
        n *= vartbl[VarIndex].dims[i] + 1 - mmb_options.base;
    }
    *bytes = n * ((*type & T_STR) ? *size + 1 : 8);
    return base;
}

/**
 * b%() = a%(), as on the PicoMite: the whole array copied, both of one type
 * (strings of one maximum length) and with as many elements, whatever the
 * dimensions.
 */
static void let_array(const char *lhs, const char *rhs) {
    skipspace(rhs);
    const char *end = rhs;
    while (*end && *end != '\'') end++;
    if (!let_is_whole_array(rhs, end)) ERROR_SYNTAX;
    int dtype, dsize, stype, ssize;
    int64_t dbytes, sbytes;
    char *dst = let_whole_array(lhs, &dtype, &dsize, &dbytes);
    if (vartbl[VarIndex].type & T_CONST) error_throw_legacy("Cannot change a constant");
    const char *src = let_whole_array(rhs, &stype, &ssize, &sbytes);
    if (dtype != stype || ((dtype & T_STR) && dsize != ssize)) error_throw_legacy("Arrays must be the same type");
    if (dbytes != sbytes) error_throw_legacy("Array size mismatch");
    memmove(dst, src, dbytes);
}

void cmd_let(void) {
    int t = 0x0;
    MMFLOAT f = 0.0;
    MMINTEGER i64 = 0;
    char *s = NULL;

    const char *p1 = cmdline;

    // search through the line looking for the equals sign
    while (*p1 && tokentbl_read(&p1) != tokenEQUAL) { }
    if(!*p1) error_throw_legacy("Unknown command");

    p1 -= tokensize(tokenEQUAL);

    // check that we have a straight forward variable
    const char *p2 = skipvar(cmdline, false);
    skipspace(p2);
    if (p1 != p2) ERROR_SYNTAX;

    if (let_is_whole_array(cmdline, p1)) {
        let_array(cmdline, p1 + tokensize(tokenEQUAL));
        return;
    }

    // create the variable and get the length if it is a string
    char *pvar = findvar(cmdline, V_FIND);
    int size = vartbl[VarIndex].size;
    if(vartbl[VarIndex].type & T_CONST) error_throw_legacy("Cannot change a constant");

    // step over the equals sign, evaluate the rest of the command and save in the variable
    p1 += tokensize(tokenEQUAL);
    if(vartbl[VarIndex].type & T_STR) {
        t = T_STR;
        p1 = evaluate(p1, &f, &i64, &s, &t, false);
        if(*s > size) error_throw_legacy("String too long");
        Mstrcpy(pvar, s);
    }
    else if(vartbl[VarIndex].type & T_NBR) {
        t = T_NBR;
        p1 = evaluate(p1, &f, &i64, &s, &t, false);
        if(t & T_NBR)
            (*(MMFLOAT *)pvar) = f;
        else
            (*(MMFLOAT *)pvar) = (MMFLOAT)i64;
    } else {
        t = T_INT;
        p1 = evaluate(p1, &f, &i64, &s, &t, false);
        if(t & T_INT)
            (*(MMINTEGER *)pvar) = i64;
        else
            (*(MMINTEGER *)pvar) = FloatToInt64(f);
    }
    checkend(p1);
}
