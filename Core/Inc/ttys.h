#ifndef _TTYS_H_
#define _TTYS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/_intsup.h>


//UART numbering is based on the MCU definition
enum ttys_instance_id{
    TTYS_INSTANCE_UART1,
    TTYS_INSTANCE_UART2, //File descriptor 1 (stdin/stdout)
    TTYS_INSTANCE_UART6,

    TTYS_NUM_INSTANCES
}

#define TTYS_RX_BUF_SIZE 80
#define TTYS_TX_BUF_SIZE 1024

struct ttys_cfg {
    bool create_stream;         //Integrate with stream API
    bool send_cr_after_nl;      //Return carrige is automatically set after a new line
};


// Core module interface functions 
int32_t ttys_get_def_cfg(enum ttys_instance_id instance_id, struct ttys_cfg* cfg);
int32_t ttys_start(enum ttys_instance_id instance_id, struct ttys_cfg* cfg);
int32_t ttys_start(enum ttys_instance_id instance_id);

//Other APIs
int32_t ttys_putc(enum ttys_instance_id instance_id, char c);
int32_t ttys_getc(enum ttys_instance_id instance_id, char* c);
int ttys_get_fd(enum ttys_instance_id instance_id);
FILE* ttys_get_stream(enum ttys_instance_id instance_id);

#endif