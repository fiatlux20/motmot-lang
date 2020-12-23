#ifndef _JITCALC_ERROR_H_
#define _JITCALC_ERROR_H_

void init_error_handler();
void report_error(const char *type, const char *message, ...);

#endif /* _JITCALC_ERROR_H_ */
