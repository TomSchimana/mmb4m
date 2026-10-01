[← Documentation](README.md)

# Limits

A program gets 128 MB for variables, arrays and strings and 1 MB for its code, the figures of MMBasic for Windows. These are MMBasic's limits, not the Mac's.

| | MMB4M |
| --- | --- |
| variables, arrays and strings | 128 MB |
| program code | 1 MB |
| upper bound of one array dimension | 2147483647 |
| variables | 2048 |
| nested `FOR` and `DO` loops | 128 each |
| open files | 128 |
| entries `FILES` lists | 2048 |
| `#DEFINE` entries | 256 |
| sprite layers | 10 |
| length of a string, and of a path | 255 characters |

String operations and the arguments of a `SUB` or `FUNCTION` call take their memory from the bottom, variables and arrays from the top, as on the PicoMite. So a program holding arrays of many megabytes stays fast. A string parameter or a `LOCAL` variable is a variable, though, and a call that has one runs noticeably slower beside such arrays.
