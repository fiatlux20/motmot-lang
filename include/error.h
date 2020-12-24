/** @file error.h */
#ifndef _ERROR_H_
#define _ERROR_H_

#include <stdio.h>
#include <stdarg.h>

/**
 * Prints an error to stderr in the format: "ErrorType: message", where
 * message is a format string.
 *
 * @param error_type A string identifying the type of the error.
 * @param message A message explaining the error.
 * @param ... Variadic arguments to be passed to vprintf.
 */
void report_error(const char *error_type, const char *message, ...);

#endif /* _ERROR_H_ */
