#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__linux__)
#define __USE_GNU
#include <signal.h>
#include <sys/ucontext.h>
#elif defined(__APPLE__)
#error "idk what to use for apple"
#endif

#include "function.h"
#include "error.h"

static FunctionTable *function_table;

static void sigseg_handler(int signum, siginfo_t *info, void *ucontext) {
    ucontext_t *context = (ucontext_t*) ucontext;
    unsigned long reg_rip = context->uc_mcontext.gregs[REG_RIP];
    report_error("FatalError", "Received SIGSEGV at 0x%016lx", reg_rip);

    if (reg_rip > (unsigned long) function_table->executable_memory &&
        reg_rip < (unsigned long) ((char*) function_table->executable_memory + function_table->memory_size)) {
        unsigned long offset = reg_rip - (unsigned long) function_table->executable_memory;
        fprintf(stderr, "Offset +0x%lx from beginning of JIT code memory page.\n\n", offset);
        fprintf(stderr, "-----------------------------------------------\n");

        long low  = ((offset - 0x20) / 0x10) * 0x10;
        long high = ((offset + 0x20) / 0x10) * 0x10;
        if (low <= 0) {
            low = 0;
        }
        if (high >= function_table->memory_size) {
            high = function_table->memory_size - 1;
        }

        fprintf(stderr, "0x%016lx to 0x%016lx\n", (unsigned long) function_table->executable_memory + low, (unsigned long) function_table->executable_memory + high - 1);
        dump_memory_range(function_table, low, high);

        // if (offset % 16 <= 8) {
        //     for (int i = 0; i < offset % 16; i++) {
        //         fprintf(stderr, "   ");
        //     }
        //     fprintf(stderr, "^-- executable_memory + 0x%lx\n", offset);
        // } else {
        //     for (int i = 0; i < (offset % 16) - 8; i++) {
        //         fprintf(stderr, "   ");
        //     }
        //     fprintf(stderr, "     executable_memory --^ + 0x%lx\n", offset);
        // }
    }

    exit(1);
}

void init_error_handler(FunctionTable *functions) {
    struct sigaction action;
    function_table = functions;
    sigaction(SIGSEGV, NULL, &action);
    action.sa_sigaction = sigseg_handler;
    action.sa_flags |= SA_SIGINFO;
    sigaction(SIGSEGV, &action, NULL);
}

void report_error(const char *error_type, const char *fmt, ...) {
    va_list(args);
    fprintf(stderr, "\033[0;31m%s\033[0m: ", error_type);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputs("\n", stderr);
}
