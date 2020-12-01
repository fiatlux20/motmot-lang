#ifndef _ERROR_H_
#define _ERROR_H_

#include <stdio.h>
#include <stdarg.h>

void report_error(const char *error_type, const char *message, ...);

#endif /* _ERROR_H_ */
