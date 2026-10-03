/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_end.c

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
#include "../common/exit_codes.h"

#include <string.h>

/** The command END "cmd$" leaves for the prompt; main() runs it once the program has ended. */
char mmb_end_command[STRINGSIZE] = { 0 };

/**
 * END [exitcode | cmd$ | NOEND]
 *
 * As on the PicoMite, a SUB MM.END runs first unless NOEND is given; without
 * one, END cmd$ leaves cmd$ to be run at the prompt after the program. An
 * exit code is MMB4L's own and holds either way.
 */
void cmd_end(void) {
    static bool in_mm_end = false;
    getargs(&cmdline, 1, DELIM_COMMA);
    const bool noend = (argc == 1) && checkstring(argv[0], "NOEND");
    mmb_state.exit_code = EX_OK;
    mmb_end_command[0] = '\0';
    if (mmb_profiling && CurrentLinePtr) profile_report();
    if (argc == 1 && !noend) {
        MMFLOAT f = 0.0;
        MMINTEGER i = 0;
        char *s = NULL;
        int t = T_NOTYPE;
        evaluate(argv[0], &f, &i, &s, &t, false);
        if (t & T_STR) {
            memcpy(mmb_end_command, s + 1, (unsigned char) s[0]);
            mmb_end_command[(unsigned char) s[0]] = '\0';
        } else {
            // the value is already there, a second evaluation would run a FUNCTION twice
            const MMINTEGER code = (t & T_INT) ? i : FloatToInt64(f);
            if (code < 0 || code > 255) error_throw_legacy("% is invalid (valid is 0 to 255)", code);  // as getint() says it
            mmb_state.exit_code = (uint8_t) code;
        }
    }
    if (!noend && !in_mm_end && FindSubFun("MM.END", kSub) >= 0) {
        mmb_end_command[0] = '\0';  // as on the PicoMite, MM.END takes the place of cmd$
        in_mm_end = true;
        ExecuteProgram("MM.END\0");
    }
    in_mm_end = false;
    longjmp(mark, JMP_END);
}
