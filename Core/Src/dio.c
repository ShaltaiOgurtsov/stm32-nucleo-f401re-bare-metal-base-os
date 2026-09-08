#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/_intsup.h>
#include <sys/stat.h>

#include "cmd.h"
#include "dio.h"
#include "log.h"
#include "module.h"
#include "stm32f4xx_ll_gpio.h"


static int32_t cmd_dio_status(int32_t argc, const char** argv);
static int32_t cmd_dio_get(int32_t argc, const char** argv);
static int32_t cmd_dio_set(int32_t argc, const char** argv);

static struct dio_cfg* cfg;

static struct cmd_cmd_info cmds[] = {
    {
        .name = "status",
        .func = cmd_dio_status,
        .help = "Get module status, usage: dio status",
    },
       {
        .name = "get",
        .func = cmd_dio_get,
        .help = "Get input value, usage: dio get <input-name>",
    },
    {
        .name = "set",
        .func = cmd_dio_set,
        .help = "Set output value, usage: dio set <output-name> {0|1}",
    },
};


static int32_t log_level = LOG_DEFAULT;

static struct cmd_client_info cmd_info = {
    .name = "dio", 
    .num_cmds = ARRAY_SIZE(cmds),
    .cmds = cmds,
    .log_level_ptr = &log_level,
};



int32_t dio_init(struct dio_cfg* _cfg){
    uint32_t idx;
    const struct dio_in_info* dii;
    const struct dio_out_info* dio;

    cfg = _cfg;

    for (idx = 0; idx < cfg->num_inputs; idx++){
        dii = &cfg->inputs[idx];
        LL_GPIO_SetPinPull(dii->port, dii->pin, LL_GPIO_MODE_INPUT);
    }
    for (idx = 0; idx < cfg->num_outputs; idx++){
        doi = &cfg->outputs[idx];
        LL_GPIO_SetPinSpeed(doi->port, doi->pin, doi->speed);
        LL_GPIO_SetPinOutputType(doi->port, doi->pin,  doi->output_type);
        LL_GPIO_SetPinPull(doi->port, doi->pin, doi->pull);
        LL_GPIO_SetPinMode(doi->port, doi->pin, LL_GPIO_MODE_OUTPUT);
    }

    return 0;
}

int32_t dio_start(void){
    int32_t result;

    result = cmd_register(&cmd_info);
    if (result < 0){
        log_error("dio_start: cmd error %d\n", result);
        return MOD_ERR_RESOURCE;
    }

    return 0;
}


int32_t dio_get(uint32_t din_idx)   
{
    if (din_idx >= cfg->num_inputs){
        return MOD_ERR_ARG;
    }

    return LL_GPIO_IsInputPinSet(cfg->inputs[dio_idx].port, 
                                cfg->inputs[din_idx].pin) ^
            cfg->inputs[din_idx].invert;
}


int32_t dio_get_out(uint32_t dout_idx){
    if (dout_idx >= cfg->num_outputs){
        return MOD_ERR_ARG;
    }

    return LL_GPIO_IsOutputPinSet(cfg->outputs[dout_idx].port,
                                cfg->outputs[dout_idx].pin) ^
    cfg->outputs[dout_idx].invert;
}


int32_t dio_set(uint32_t dout_idx, uint32_t value){
    if (dout_idx >= cfg->num_inputs){
        return MOD_ERR_ARG;
    }
    if (value ^ cfg->outputs[dout_idx].invert){
         LL_GPIO_SetOutputPin(cfg->outputs[dout_idx].port,
                             cfg->outputs[dout_idx].pin);
    } else {
        LL_GPIO_ResetOutputPin(cfg->outputs[dout_idx].port,
                            cfg->outputs[dout_idx].pin);
    }

    return 0;
}




int32_t dio_get_num_in(void){
    return  cfg == NULL ? MOD_ERR_RESOURCE : cfg->num_inputs;
}


int32_t dio_get_num_out(void)
{
    return cfg == NULL ? MOD_ERR_RESOURCE : cfg->num_outputs;
}


static int32_t cmd_dio_status(int32_t argc, const char** argv)
{
    uint32_t idx;
    
    printf("Inputs:\n");
    for (idx = 0; idx < cfg->num_inputs; idx++)
        printf("  %2lu: %s = %ld\n", idx, cfg->inputs[idx].name, dio_get(idx));
    

    printf("Outputs:\n");
    for (idx = 0; idx < cfg->num_outputs; idx++)
        printf("  %2lu: %s = %ld\n", idx, cfg->outputs[idx].name,
               dio_get_out(idx));

    return 0;
}



static int32_t cmd_dio_get(int32_t argc, const char** argv){
    uint32_t idx;
    struct cmd_arg_val arg_vals[1];

    if (cmd_parse_args(argc-2, argv+2, "s", arg_vals) != 1){
        return  MOD_ERR_BAD_CMD;
    }

    for (idx = 0; idx < cfg->num_inputs; idx++){
        if(strcasecmp(arg_vals[0].val.s, cfg->inputs[idx].name) == 0){
            break;
        }
    }

    if (idx < cfg->num_inputs){
        printf("%s = %ld\n", cfg->inputs[idx].name, dio_get(idx));
        return 0;
    }

    for(idx = 0; idx < cfg->num_outputs; idx++){
        if (strcasecmp(arg_vals[0].val.s, cfg->outputs[idx].name) == 0){
            break;
        }
    }

    if (idx < cfg->num_outputs){
        printf("%s %ld\n", cfg->outputs[idx].name, dio_get_out(idx));
        return 0;
    }

    printf("Invalid input/output name '%s'\n", arg_vals[0].val.s);
    return MOD_ERR_ARG;
}


static int32_t cmd_dio_set(int32_t argc, const char** argv){
    uint32_t idx;
    struct cmd_arg_val arg_vals[2];
    uint32_t value;

    if (cmd_parse_args(argc-2, argv+2, "su", arg_vals) != 2)
        return MOD_ERR_BAD_CMD;

    for (idx = 0; idx < cfg->num_outputs; idx++){
        if (strcasecmp(arg_vals[0].val.s, cfg->outputs[idx].name) == 0){
            break;
        }
    }
    if(idx >= cfg->num_outputs){
        printf("Invalid dio name '%s'\n", arg_vals[0].val.s);
        return MOD_ERR_ARG;
    }


    value = arg_vals[1].val.u;
    if (value != 0 && value != 1) {
        printf("Invalid value '%s'\n", argv[3]);
        return MOD_ERR_ARG;
    }
    return dio_set(idx, value);
}