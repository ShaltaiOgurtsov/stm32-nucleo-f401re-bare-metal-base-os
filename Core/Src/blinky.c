#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/_intsup.h>
#include <sys/stat.h>
#include <time.h>

#include "cmd.h"
#include "dio.h"
#include "log.h"
#include "module.h"

#include "blinky.h"

struct blinky_state{
    struct blinky_cfg cfg;
    uint8_t valid_dio;
    uint8_t led_on;
};

static int32_t cmd_blinky_status(int32_t argc, const char** argv);
static int32_t cmd_blinky_on(int32_t argc, const char** argv);
static int32_t cmd_blinky_off(int32_t argc, const char** argv);
static int32_t cmd_blinky_toggle(int32_t argc, const char** argv);


static struct blinky_state state;

static struct cmd_cmd_info cmds[] = {
    {
        .name = "status",
        .func = cmd_blinky_status,
        .help = "Get module status, usage: blinky status",
    },
    {
        .name = "on",
        .func = cmd_blinky_on,
        .help = "Turn LED on, usage: blinky on",
    },
    {
        .name = "off",
        .func = cmd_blinky_off,
        .help = "Turn LED off, usage: blinky off",
    },
    {
        .name = "toggle",
        .func = cmd_blinky_toggle,
        .help = "Toggle LED, usage: blinky toggle",
    },
};

static int32_t log_level = LOG_DEFAULT;

static struct cmd_client_info cmd_info = {
    .name = "blinky",
    .num_cmds = ARRAY_SIZE(cmds),
    .cmds = cmds,
    .log_level_ptr = &log_level,
};



int32_t blinky_cfg_get_cfg(struct blinky_cfg* cfg){
    if (cfg == NULL){
        return MOD_ERR_ARG;
    }

    memset(cfg, 0, sizeof(*cfg));
    cfg->dout_idx = 0;

    return 0;
}



int32_t blinky_init(struct blinky_cfg *cfg){
    log_debug("In blinky init()\n");

    if (cfg == NULL){
        return MOD_ERR_ARG;
    }

    memset(&state, 0, sizeof(state));
    state.cfg = *cfg;
    state.led_on = false;

    return 0;
}


int32_t blinky_set_state(uint8_t on){
    if (!state.valid_dio){
        return  MOD_ERR_STATE;
    }

    dio_set(state.cfg.dout_idx, on);
    state.led_on = on;

    return 0;
}


static int32_t cmd_blinky_status(int32_t argc, const char** argv){
    printf("led=%s dout_idx=%lu\n",
             state.led_on ? "ON" : "OFF", state.cfg.dout_idx);

    return 0;
}


static int32_t cmd_blinky_on(int32_t argc, const char** argv){
    int32_t result = blinky_set_state(1);

    if (result < 0){
        printf("Error: LED not available\n");
        return result;
    }

    printf("LED on\n");

    return 0;
}


static int32_t cmd_blinky_off(int32_t argc, const char** argv){
    int32_t result = blinky_set_state(0);
    if (result < 0){
        printf("Error: LED not available\n");
        return  result;
    }

    printf("LED off\n");
    return 0;
}



static int32_t cmd_blinky_toggle(int32_t argc, const char** argv){
    int32_t result = blinky_set_state(!state.led_on);
    if (result < 0){
        printf("Error: LED not available\n");
        return result;
    }

    printf("LED %s\n", state.led_on ? "on" : "off");

    return 0;
}