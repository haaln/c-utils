#include "log.h"

#include "stdio.h"
#include <stdarg.h>

// Below are color codes for stdout colors.
#define NRM "\x1B[0m"

#define LOG_BLK "\x1B[30m"
#define LOG_RED "\x1B[31m"
#define LOG_GRN "\x1B[32m"
#define LOG_YEL "\x1B[33m"
#define LOG_BLU "\x1B[34m"
#define LOG_MAG "\x1B[35m"
#define LOG_CYN "\x1B[36m"
#define LOG_WHT "\x1B[37m"

#define LOG_BLK_BG "\x1B[40m"
#define LOG_RED_BG "\x1B[41m"
#define LOG_GRN_BG "\x1B[42m"
#define LOG_YEL_BG "\x1B[43m"
#define LOG_BLU_BG "\x1B[44m"
#define LOG_MAG_BG "\x1B[45m"
#define LOG_CYN_BG "\x1B[46m"
#define LOG_WHT_BG "\x1B[47m"

#define BOLD  "\x1B[1m"
#define ULINE "\x1B[4m"


// TODO:: add functionality to be able to different outputs, stderr/files etc...

static int log_printf(const char *level, const char *format, va_list args)
{
    if(int err = fprintf(stdout, "[%s] ", level) < 0) {
        return err;
    }
    return vfprintf(stdout, format, args);
}

void __log_trace(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf("TRACE", format, args);
    va_end(args);
}

void __log_debug(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(LOG_CYN "DEBUG" NRM, format, args);
    va_end(args);
}

void __log_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(LOG_GRN "INFO" NRM, format, args);
    va_end(args);
    return;
}

void __log_warn(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(LOG_YEL "WARN" NRM, format, args);
    va_end(args);
    return;
}

void __log_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(LOG_RED "ERROR" NRM, format, args);
    va_end(args);
    return;
}

void __log_critical(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_printf(BOLD LOG_RED "CRITICAL" NRM, format, args);
    va_end(args);
    return;
}

