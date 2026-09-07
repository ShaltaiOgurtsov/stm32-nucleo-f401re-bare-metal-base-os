#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/_intsup.h>

#include "cmd.h"
#include "console.h"
#include "stm32f410rx.h"
#include "ttys.h"
#include "log.h"
#include "module.h"
#include "app_main.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"

enum main_u16_pms{
    CNT_INIT_ERR,
    CNT_START_ERR,
    CNT_RUN_ERR,

    NUM_U16_PMS
};

static int32_t cmd_main_status();

static int32_t log_level = LOG_DEFAULT;

static struct cmd_cmd_info cmds[] = {
    {
        .name = "status",
        .func = cmd_main_status,
        .help = "Get main status, usage: main status [clear]",
    },
};

static uint16_t cnts_u16[NUM_U16_PMS];

static const char* cnts_u16_names[NUM_U16_PMS] = {
    "init err",
    "start err",
    "run err",
};

static struct cmd_client_info cmd_info = {
    .name = "main",
    .num_cmds = ARRAY_SIZE(cmds),
    .cmds = cmds,
    .log_level_ptr = &log_level,
    .num_u16_pms = NUM_U16_PMS,
    .u16_pms = cnts_u16,
    .u16_pm_names = cnts_u16_names,
};

void uart_clear_and_home(USART_TypeDef *USARTx){
    const char *ansi_cls = "\033[2J\033[H";
    while (*ansi_cls) {
        while (LL_USART_IsActiveFlag_TXE(USARTx)) {
            LL_USART_TransmitData8(USARTx, (uint8_t)*ansi_cls++);
        }
    }
}



void app_main(void){
    int32_t result;
    struct console_cfg console_cfg;
    struct ttys_cfg ttys_cfg;

    // uart_clear_and_home(USART2);


    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\033[2J\033[H");

    printf("\nInit: Init modules\n");


    // INITIALIZING MODULES

    // Initialize UART2 
    result = ttys_get_def_cfg(TTYS_INSTANCE_UART2, &ttys_cfg);
    if (result < 0){
        INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
    } else {
        result = ttys_init(TTYS_INSTANCE_UART2, &ttys_cfg);
        if (result < 0){
            INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
        }
    }



    result = ttys_get_def_cfg(TTYS_INSTANCE_UART6, &ttys_cfg);
    if (result < 0){
        INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
    } else {
        result = ttys_init(TTYS_INSTANCE_UART6, &ttys_cfg);
        if (result < 0){
            INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
        }
    }



    result = cmd_init(NULL);
    if (result < 0){
        INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
    }


    result = console_get_def_cfg(&console_cfg);
    if (result < 0){
        INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
    } else {
        result = console_init(&console_cfg);
        if (result < 0){
            INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
        }
    }

    // STARTING MODULES

    printf("Init: start modules\n");

    result = ttys_start(TTYS_INSTANCE_UART2);
    if (result < 0){
        INC_SAT_U16(cnts_u16[CNT_START_ERR]);
    }

    result = ttys_start(TTYS_INSTANCE_UART6);
    if (result < 0){
        INC_SAT_U16(cnts_u16[CNT_START_ERR]);
    }


    result = cmd_register(&cmd_info);
    if (result < 0){
        INC_SAT_U16(cnts_u16[CNT_START_ERR]);
    }

    

    printf("Init: Enter super loop\n");

    while (1) {

        result = console_run();
        if (result < 0){
            INC_SAT_U16(cnts_u16[CNT_RUN_ERR]);
        }
    }
}




static int32_t cmd_main_status(int32_t argc, const char** argv){
    bool clear = false;
    bool bad_arg = false;

    if (argc == 3){
        if (argc == 3){
            if (strcasecmp(argv[2], "clear") == 0){
                clear = true;
            } else {
                bad_arg = true;
            }
        } else if (argc > 3){
            bad_arg = true;
        }
    } 


    if (bad_arg){
        printf("Invalid argument");
        return MOD_ERR_ARG;
    }

    if (clear){
        printf("Clearing loop stat\n");
    }

    return 0;

}