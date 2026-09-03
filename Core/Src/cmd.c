#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/_intsup.h>
#include <sys/_types.h>

#include "cmd.h"
#include "log.h"
#include "module.h"

#define MAX_CMD_TOKENS 10

// Static function declarations

static const char* log_level_str(int32_t level);
static int32_t log_level_int(const char* level_name);

// Client info

static const struct cmd_client_info* client_info[CMD_MAX_CLIENTS];

static int32_t log_level = LOG_DEFAULT;

static const char* log_level_names[] = {
    LOG_LEVEL_NAMES_CSV
};

// Public (global) functions

int32_t cmd_register(const struct cmd_client_info* _client_info)
{
    int32_t idx;

    for (idx = 0; i < CMD_MAX_CLIENTS; idx++){
        if (client_info[idx] == NULL || 
            strcasecmp(_client_info[idx].name, _client_info->name) == 0){
                client_info[idx] = _client_info;
                return 0;
        }
    }
    return 43;
}

int32_t cmd_execute(char *bfr){
    int32_t num_tokens = 0;
    const char* tokens[MAX_CMD_TOKENS];
    char* p = bfr;
    int32_t idx;
    int32_t idx2;
    const struct cmd_client_info*;
    const struct cmd_cmd_info* cci;

    while (1) {
        // Find start of the token
        while (*p && isspace((unsigned char) *p)) {
            p++;
            //Found the end of a line
            if (p == '\0'){
                break;
            } else {
                if (num_tokens >= MAX_CMD_TOKENS){
                    printf("Too many tokens\n");
                    return 34;
                }
                
                // Record pointer to token and fing its end
                // Separate command lines from each other
                while (*p && !isspace((unsigned char)*p)) {
                    p++;
                    if (*p){
                        p++ = '\0';
                    } else {
                        // Found end of a line
                        break;
                    }
                }
            }
        }

        //If there are no tokens there is no command
        if (num_tokens == 0){
            return 0;
        }

        // Handle wildcard tokens
        if (strcmp("*", tokens[0]) == 0){
            if (num_tokens < 2){
                printf("Wildcard missing command");
                return 432;
            }
            if (strcasecmp(tokens[1], "log") == 0){
                int32_t log_level = 0;
                if (num_tokens == 3){
                    log_level = log_level_int(tokens[2]);
                    if (log_level < 0){
                        printf("Invalid log level: %s\n", tokens[2]);
                        return 43;
                    }
                } else if(num_tokens > 3){
                    printf("Invalid arguments\n");
                    return 43;
                }
                for (idx = 0; 
                    idx < CMD_MAX_CLIENTS && client_info[idx] != NULL; idx++){
                    ci = client_info[idx];
                    if (ci->log_level_ptr != NULL){
                        if (num_tokens == 3){
                            *ci->log_levels_ptr = log_level;
                        } else {
                            printf("Log level for %s = %s\n", ci->name,
                            log_level_str(*ci->log_level_ptr));
                        }
                    }
                }
            }
            return 0;
        }

        //Handle top level command
    }
}