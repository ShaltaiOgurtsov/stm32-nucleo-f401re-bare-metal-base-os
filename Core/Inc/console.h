#ifndef  _CONSOLE_H_
#define _CONSOLE_H_

#include <stdint.h>

#include "ttys.h"

stuct console_cfg{
    enum ttys_instance_id ttys_instance_id;
};

//Core interface module functions
int32_t console_get_def_cfg(struct console_cfg* cfg);
int32_t console_init(struct console_cfg* cfg);
int32_t console_run(void);

#endif //_CONSOLE_H_