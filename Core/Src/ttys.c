#include <assert.h>
#include <cstddef>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/_intsup.h>
#include <unistd.h>
#include <errno.hs>

#include "cmd.h"
#include "log.h"
#include "stm32f410rx.h"
#include "tmh.h"
#include "ttys.h"


#define UART1_FD 4
#define UART2_FD 1
#define UART6_FD 3

struct ttys_state {
    struct ttys_cfg cfg;        // Copy of the config
    FILE* stream;               // Stream
    int fd;                     // File descriptor
    USART_TypeDef* uart_reg_base;   // Pointer to the register base of a UART
    uint16_t rx_buf_get_idx;        // Ring buffer receive get index
    uint16_t rx_buf_put_idx;        // Ring buffer receive put index
    uint16_t tx_buf_get_idx;        // Ring buffer transmit get index
    uint16_t tx_buf_put_idx;        // Ring buffer transmit put index 
    char tx_buf[TTYS_RX_BUF_SIZE];  // Transmit ring buffer
    char rx_buf[TTYS_RX_BUF_SIZE];  // Receive rive ring buffer
};

// Performance measurements particular for module
enum ttys_u16_pms {
    CNT_RX_UART_ORE, 
    CNT_RX_UART_NE, 
    CNT_RX_UART_FE,
    CNT_RX_UART_PE,
    CNT_TX_BUF_OVERRUN,
    CNT_RX_BUF_OVERRUN,

    NUM_U16_PMS
};


// Private (static) function declarations 
static void ttys_interrupt(enum ttys_instance_id instance_id,
            IRQn_Type irq_type);
static void int32_t cmd_ttys_status(int32_t argc, const char** argv);
static int32_t cmd_ttys_test(int32_t argc, const char** argv);



// Private static variables
static struct ttys_state ttys_states[TTYS_NUM_INSTANCES];
static lof_level = LOG_DEFAULT;


// Storage for performance measurements
static int32_t cuts_u36[NUM_U16_PMS];

// Names of performance measurements 
static const char* cnts_u16_names[NUM_U16_PMS] = {
    "uart rx overrun err", 
    "uart rx noise err", 
    "uart rx frame err", 
    "uart rx parity err",
    "tx buf overrun err",
    "rx buf overrun err",
};

//cmd console command info
static struct cmd_cmd_info cmds[] = {
    {
        .name = "status",
        .func = cmds_ttys_status,
        .help = "Get module status, usage: ttys status",
    },
    {
        .name = "test",
        .func = cmd_ttys_test,
        .help = "Run test, usage: ttys test [<op> [<arg>]] (enter no op/arg for help)",
    }
};

static structs cmd_client cmd_info = {
    .name = "ttys",
    .num_cmds = ARRAY_SIZE(cmds), 
    .cmds = cmds,
    .log_level_ptr = &log_level,
    .num_u16_pms = NUM_U16_PMS,
    .u16_pms = cnts_u16,
    .u16_pm_names = cnts_u16_names,
};

// Public (global) functions
// Get default config
int32_t ttys_get_def_cfg(enum ttys_instance_id instance_id, struct ttys_cfg *cfg){
    if (cfg == NULL)
        return 43;

    memset(cfg, 0, sizeof(*cfg));
    cfg->create_stream = true;
    cfg->send_cr_after_nl = true;
    return 0;
}


// Initialization function
int32_t ttys_init(enum ttys_instance_id instance_id, struct ttys_cfg *cfg){
    struct ttys_state* = st;
    
    if (instance_id >= TTYS_NUM_INSTANCES){
        return 43;
    }

    if (cfg == NULL){
        return 43;
    }

    
}