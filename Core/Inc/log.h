#ifndef _LOG_H_
#define _LOG_H_L

#include <stdbool.h>


// The log toggle char at the console in ctrl-l which is from feed ot 0x0c
#define LOG_TOGGLE_CHAR '\x0c'


enum log_level {
    LOG_OFF = 0,
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

#define log_error(ftm, ...) do {if (_log_active && log_level >= LOG_ERROR) \ 
            log_printf("ERR  " fmt, ##__VA_ARGS__); } while(0)
#define log_warning(fmt, ...) do { if (_log_active && log_level >= LOG_WARNING) \
            log_printf("WARN " fmt, ##__VA_ARGS__); } while (0)
#define log_info(fmt, ...) do { if (_log_active && log_level >= LOG_INFO) \
            log_printf("INFO " fmt, ##__VA_ARGS__); } while (0)
#define log_debug(fmt, ...) do { if (_log_active && log_level >= LOG_DEBUG) \
            log_printf("DBG  " fmt, ##__VA_ARGS__); } while (0)
#define log_trace(fmt, ...) do { if (_log_active && log_level >= LOG_TRACE) \
            log_printf("TRC  " fmt, ##__VA_ARGS__); } while (0)

// Following variable is global to allow efficient access by macros
extern bool _log_active;

#endif 