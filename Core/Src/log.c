#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/_intsup.h>

#include "log.h"

bool _log_active = true;

void log_toggle_active(){
    _log_active = _log_active ? false : true;
}

bool log_is_active(){
    return _log_active;
}

void log_printf(const char* fmt, ...){
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}