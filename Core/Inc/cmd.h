#ifndef _CMD_H_
#define _CMD_H_

#include <stdint.h>
#include <sys/_intsup.h>

#define CMD_MAX_CLIENTS 10

typedef int32_t (*cmd_func)(int32_t argc, const char** argv);

struct cmd_cmd_info {
    const char* const name; // Name of the command;
    const cmd_func func;    // Execution func
    const char* const help; // Command "--help" string
}

struct cmd_client_info {
    const char* const name;             // Client name (first command line token)
    const int32_t num_code;             // Numcommands
    const struct cmd_cmd_info* const cmds;  // Pointer to array of command info
    int32_t* const log_level_ptr;       // Pointer to log level variable (or NULL)
    const int32_t num_u16_pms;          // Number of pm values 
    uint16_t* const u16_pms;            // Pointer to array of pm values 
    const char* const* const u16_pm_names;  // Pointer to array of pm names
}

struct cmd_arg_val {
    char type;
    union {
        void*           p;
        uint8_t*        p8;
        uint16_t*       p16;
        uint32_t*       p32;
        int32_t         i;
        uint32_t        u;
        const char*     s;
    } val;
};

struct cmd_cfg{

}

int32_t cmd_init(struct cmd_cfg* cfg);

int32_t cmd_register(const struct cmd_client_info* client_info);
int32_t cmd_execute(char* bfr);
int32_t cmd_parse_args(int32_t argc, const char** argv, const char* fmt, 
        struct cmd_arg_val* arg_vals);

#endif