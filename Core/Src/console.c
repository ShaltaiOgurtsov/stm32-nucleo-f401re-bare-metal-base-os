#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_intsup.h>

#include "console.h"
#include "cmd.h"
#include "log.h"
#include "module.h"
#include "ttys.h"

#define PROMPT "> "

// Type defenitions 

#define CONSOLE_CMD_BUFFER_SIZE 80

struct console_state {
    struct console_cfg cfg;                     // Instance config
    char cmd_bfr[CONSOLE_CMD_BUFFER_SIZE];      // Cmd buffer
    uint32_t num_cmd_bfr_chars;                 // Number of characters in the buffer
    bool first_run_done;
};

//  Private (static) variables

static struct console_state state;
static int32_t log_level = LOG_DEFAULT;

// Public (Global) functions

int32_t console_get_def_cfg(struct console_cfg *cfg){
    if (cfg == NULL){
        return MOD_ERR_ARG;
    }

    memset(cfg, 0, sizeof(*cfg));
    cfg->ttys_instance_id = TTYS_INSTANCE_UART2;
    return 0;
}

int32_t console_init(struct console_cfg* cfg){
    if(cfg == NULL){
        return MOD_ERR_ARG;
    }

    memset(&state, 0, sizeof(state));
    state.cfg = *cfg;
    return 0;
}



int32_t console_run(void)
{
    char c;
    if (!state.first_run_done) {
        state.first_run_done = true;
        printf("%s", PROMPT);
    }
            
    while (ttys_getc(state.cfg.ttys_instance_id, &c)) {

        // Handle processing completed command line.
        if (c == '\r') {
            state.cmd_bfr[state.num_cmd_bfr_chars] = '\0';
            printf("\n");
            cmd_execute(state.cmd_bfr);
            state.num_cmd_bfr_chars = 0;
            printf("%s", PROMPT);
            continue;
        }

        // Handle backspace/delete.
        if (c == '\b' || c == '\x7f') {
            if (state.num_cmd_bfr_chars > 0) {
                // Overwrite last character with a blank.
                printf("\b \b");
                state.num_cmd_bfr_chars--;
            }

            continue;
        }

        // Handle logging on/off toggle.
        if (c == LOG_TOGGLE_CHAR) {
            log_toggle_active();
            printf("\n<Logging %s>\n", log_is_active() ? "on" : "off");
            continue;
        }

        // Echo the character back.
        if (isprint(c)) {
            if (state.num_cmd_bfr_chars < (CONSOLE_CMD_BUFFER_SIZE-1)) {
                state.cmd_bfr[state.num_cmd_bfr_chars++] = c;
                printf("%c", c);
            } else {
                // No space in buffer for the character, so ring the bell.
                printf("\a");
            }
            continue;
        }
            
    }
    return 0;
}            

