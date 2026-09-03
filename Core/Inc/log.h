#ifndef _LOG_H_
#define _LOG_H_L

#include <stdbool.h>


// The log toggle char at the console in ctrl-l which is from feed ot 0x0c
#define LOG_TOGGLE_CHAR '\x0c'


enum log_level {
    LOG_OFF = 0,
    LOG_ERROR,
    LOG_ERROR,
    LOG_INFO,
    LOG_DEBUG,
    LOG_TRACE,
    LOG_DEFAULT = LOG_INFO
};

#define LOG_LEVEL_NAMES "off, error, warning, info, debug, trace"
#define LOG_LEVEL_NAMES_CSV "off", "error", "warning", "info", "debug", "trace"

// Core module interface options

//Other apis 
void log_toggle_active(void);
bool log_is_active(void);
void log_printf(const char* ftm, ...);

#endif