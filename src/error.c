#include "error.h"

void report_error(const char *error_type, const char *fmt, ...) {
    va_list(args);
    fprintf(stderr, "\033[0;31m%s\033[0m: ", error_type);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputs("\n", stderr);
}
