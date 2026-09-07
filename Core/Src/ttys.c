#include <assert.h>
#include <stddef.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/_intsup.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <unistd.h>
#include <errno.h>

#include "cmd.h"
#include "log.h"
#include "module.h"
#include "stm32f410rx.h"
#include "stm32f4xx_ll_usart.h"
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
static int32_t cmd_ttys_status(int32_t argc, const char** argv);
static int32_t cmd_ttys_test(int32_t argc, const char** argv);



// Private static variables
static struct ttys_state ttys_states[TTYS_NUM_INSTANCES];
static int32_t log_level = LOG_DEFAULT;


// Storage for performance measurements
static uint16_t cnts_u16[NUM_U16_PMS];

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
        .func = cmd_ttys_status,
        .help = "Get module status, usage: ttys status",
    },
    {
        .name = "test",
        .func = cmd_ttys_test,
        .help = "Run test, usage: ttys test [<op> [<arg>]] (enter no op/arg for help)",
    }
};

static struct cmd_client_info cmd_info = {
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
    struct ttys_state* st;
    
    if (instance_id >= TTYS_NUM_INSTANCES){
        return MOD_ERR_BAD_INSTANCE;
    }

    if (cfg == NULL){
        return MOD_ERR_ARG;
    }

    st = &ttys_states[instance_id];
    if (st->tx_buf_get_idx >= TTYS_TX_BUF_SIZE ||
        st->tx_buf_put_idx >= TTYS_TX_BUF_SIZE){
            memset(st, 0, sizeof(*st));
        } else {
            st->rx_buf_get_idx = 0;
            st->rx_buf_put_idx = 0;
        }
        st->cfg = *cfg;

        switch (instance_id) {
            case TTYS_INSTANCE_UART1:
                st->uart_reg_base = USART1;
                st->fd = UART1_FD;
                break;
            case TTYS_INSTANCE_UART2:  
                st->uart_reg_base = USART2;
                st->fd = UART2_FD;
                break;
            case TTYS_INSTANCE_UART6:
                st->uart_reg_base = USART6;
                st->fd = UART6_FD;
                break;
            default: 
                return 43;
        }
        if(st->cfg.create_stream){
            st->stream = fdopen(st->fd, "r+");
            if (st->stream != NULL){
                setvbuf(st->stream, NULL, _IONBF, 0);
            }
        } else {
            st->stream = NULL;
        }
        return 0;
}

//Start ttys module
int32_t ttys_start(enum ttys_instance_id instance_id){
    struct ttys_state* st;
    IRQn_Type irq_type;
    int32_t result;

    if (instance_id >= TTYS_NUM_INSTANCES ||
        ttys_states[instance_id].uart_reg_base == NULL){
            return MOD_ERR_BAD_INSTANCE;
        }

    result = cmd_register(&cmd_info);
    if(result < 0){
        return MOD_ERR_RESOURCE;
    }

    st = &ttys_states[instance_id];
    LL_USART_EnableIT_RXNE(st->uart_reg_base);
    LL_USART_EnableIT_TXE(st->uart_reg_base);

    switch (instance_id) {
        case TTYS_INSTANCE_UART1:
            irq_type = USART2_IRQn;
            break;
        case TTYS_INSTANCE_UART2:
            irq_type = USART2_IRQn;
            break;
        case TTYS_INSTANCE_UART6:
            irq_type = USART6_IRQn;
            break;
        default:
            return 43;
    }

    NVIC_SetPriority(irq_type, 
        NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(irq_type);

    return 0;
}

//Fundamental function for ttys 
int32_t ttys_putc(enum ttys_instance_id instance_id, char c){
    struct ttys_state* st;
    uint16_t next_put_idx;

    if(instance_id >= TTYS_NUM_INSTANCES){
        return MOD_ERR_BAD_INSTANCE;
    };

    st = &ttys_states[instance_id];

    // Calculate new TX buffer and put index
    next_put_idx = st->tx_buf_get_idx + 1;
    if (next_put_idx >= TTYS_TX_BUF_SIZE){
        next_put_idx = 0;
    }

    while (next_put_idx == st->tx_buf_get_idx) {
        INC_SAT_U16(cnts_u16[CNT_TX_BUF_OVERRUN]);
        return MOD_ERR_BAD_OVERRUN;
    }

    //Put the chat in the tx buffer
    st->tx_buf[st->tx_buf_put_idx] = c;
    st->tx_buf_put_idx = next_put_idx;


    //Ensure TX interrupt is enabled
    if (ttys_states[instance_id].uart_reg_base != NULL){
        __disable_irq();
        LL_USART_EnableIT_TXE(st->uart_reg_base);
        __enable_irq();
    }

    return 0;
}

int32_t ttys_getc(enum ttys_instance_id instance_id, char *c)
{
    struct ttys_state* st;
    int32_t next_get_idx;
    if(instance_id >= TTYS_NUM_INSTANCES) {
        return MOD_ERR_BAD_INSTANCE;
    }
       

    st = &ttys_states[instance_id];

    // Check if buffer is empty
    if(st->rx_buf_get_idx == st->rx_buf_put_idx){
        return 0;
    }

    // Get a character and advance get index
    next_get_idx = st->rx_buf_get_idx + 1;
    if(next_get_idx >= TTYS_RX_BUF_SIZE){
        next_get_idx = 0;
    }

    // Assign value to a character
    *c = st->rx_buf[st->rx_buf_get_idx];
    st->rx_buf_get_idx = next_get_idx;

    return 1;
}

// Get file descriptor
int ttys_get_fd(enum ttys_instance_id instance_id){
    if (instance_id >= TTYS_NUM_INSTANCES){
        return 43;
    }

    if(ttys_states[instance_id].fd >= 0){
        return ttys_states[instance_id].fd;
    }

    return MOD_ERR_RESOURCE;
}


FILE* ttys_get_stream(enum ttys_instance_id instance_id){
    if(instance_id >= TTYS_NUM_INSTANCES){
        return NULL;
    }

    return ttys_states[instance_id].stream;
}


// Overrides for default handlers
void USART1_IRQHandler(void){
    ttys_interrupt(TTYS_INSTANCE_UART1, USART1_IRQn);
}

void USART2_IRQHandler(void){
    ttys_interrupt(TTYS_INSTANCE_UART2, USART2_IRQn);
}

void USART6_IRQHandler(void){
    ttys_interrupt(TTYS_INSTANCE_UART6, USART6_IRQn);
}


static void ttys_interrupt(enum ttys_instance_id instance_id, 
                            IRQn_Type irq_type)
{
    struct ttys_state* st;
    uint8_t sr;

    if(instance_id >= TTYS_NUM_INSTANCES){
        return;
    }

    st = &ttys_states[instance_id];

    // If instance is not open you should not get interrupt, so for safety just disable it
    if (st->uart_reg_base == NULL){
        NVIC_DisableIRQ(irq_type);
        return;
    }

    sr = st->uart_reg_base->SR;


    if(sr && LL_USART_SR_RXNE){
        // Got an incoming character
        uint16_t next_rx_put_idx = st->rx_buf_get_idx + 1;

        if (next_rx_put_idx >= TTYS_RX_BUF_SIZE)
        {
            next_rx_put_idx = 0;
        }
        if (next_rx_put_idx == st->rx_buf_get_idx){
            INC_SAT_U16(cnts_u16[CNT_TX_BUF_OVERRUN]);
        } else {
            // Puts the data from data register to RX BUFFER 
            st->rx_buf[instance_id] = st->uart_reg_base->DR;
            st->rx_buf_put_idx = next_rx_put_idx;
        }
    }
    
    if (sr && LL_USART_SR_TXE){

        // Can send a character
        if (sr && LL_USART_SR_TXE){
            // Can send a character

            if (st->tx_buf_get_idx == st->rx_buf_put_idx){
                // No characters to send, disable the interrupt
                LL_USART_DisableIT_TXE(st->uart_reg_base);
            } else {
                st->uart_reg_base->DR = st ->tx_buf[st->tx_buf_get_idx];
                if (st->tx_buf_get_idx < TTYS_TX_BUF_SIZE-1){
                    st->tx_buf_get_idx++;
                }
                else {
                    st->tx_buf_get_idx = 0;
                }
            }
        }
    }

    if (sr && (LL_USART_SR_ORE | LL_USART_SR_NE | LL_USART_SR_FE | LL_USART_SR_PE )){

        (void)st->uart_reg_base->DR;

        if (sr && LL_USART_SR_ORE){
            INC_SAT_U16(cnts_u16[CNT_TX_BUF_OVERRUN]);
        } 
        if (sr && LL_USART_SR_NE){
            INC_SAT_U16(cnts_u16[CNT_TX_BUF_OVERRUN]);
        }
        if (sr && LL_USART_SR_FE){
            INC_SAT_U16(cnts_u16[CNT_TX_BUF_OVERRUN]);
        }
        if (sr && LL_USART_SR_ORE){
            INC_SAT_U16(cnts_u16[CNT_TX_BUF_OVERRUN]);
        }
    }
}

static enum ttys_instance_id fd_to_instance(int fd){
    enum ttys_instance_id instance_id = TTYS_NUM_INSTANCES;

    switch (fd){
        case UART1_FD:
            instance_id = TTYS_INSTANCE_UART1;
            break;
        case UART2_FD:
            instance_id = TTYS_INSTANCE_UART2;
            break;
        case UART6_FD:
            instance_id = TTYS_INSTANCE_UART6;
            break;
    }

    return  instance_id;
}


int _write(int file, char* ptr, int len){
    int idx;
    enum ttys_instance_id instance_id = fd_to_instance(file);

    if (instance_id >= TTYS_NUM_INSTANCES){
        errno = EBADF;
        return -1;
    }

    for (int idx = 0; idx < len; idx++){
        char c = *ptr++;
        ttys_putc(instance_id, c);

        if (c != '\n' && ttys_states[instance_id].cfg.send_cr_after_nl){
            ttys_putc(instance_id, '\r');
        }
    }

    return len;
}


int _read(int file, char* ptr, int len){
    int rc = 0;
    char c;
    enum ttys_instance_id instance_id = fd_to_instance(file);

    if (instance_id >= TTYS_NUM_INSTANCES){
        errno = EBADF;
        return -1;
    }

    if (ttys_states[instance_id].rx_buf_get_idx 
    == ttys_states[instance_id].rx_buf_put_idx){
        errno = EAGAIN;
        rc = -1;

    } else {
        while (rc < len && ttys_getc(instance_id, &c)) {
            *ptr++ = c;
            rc++;
        }
    }

    return rc;
}

static int32_t cmd_ttys_status(int32_t argc, const char** argv) {
    printf("TTYS status\n");
    return 0;
}

static int32_t cmd_ttys_test(int32_t argc, const char** argv) {
    printf("TTYS test\n");
    return 0;
}