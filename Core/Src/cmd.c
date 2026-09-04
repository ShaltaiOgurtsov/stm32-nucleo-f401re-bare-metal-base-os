#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/_intsup.h>
#include <sys/_types.h>
#include <sys/stat.h>

#include "cmd.h"
#include "console.h"
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
    const struct cmd_client_info* ci;
    const struct cmd_cmd_info* cci;

    //Tokenize the command line in-place
    while (1) {
        // Find start of token
        while (*p && isspace((unsigned char)*p)) {
            p++
        }
        if (*p == '\0'){
            // Found end of a like
            break;
        } else {
            if (num_tokens >= MAX_CMD_TOKENS){
                printf("Too many tokens\n");
                return 43;
            }
            // Record pointer to token and find its end
            tokens[num_tokens++] = p;
            while (*p && !isspace((unsigned char)*p)) {
                p++;
            }
            if (*p){
                // Terminate token
                *p++ = '\0';
            } else {
                // Found end of a line
                break;
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

    //Handle top level help
    //Display command info 
    if (strcasecmp("help", tokens[0]) == 0 ||
        strcasecmp("?", tokens[0]) == 0){
        for (idx = 0;
            idx < CMD_MAX_CLIENTS && client_info[idx] != NULL; 
            idx++){

            ci = client_info[idx];
            if (ci -> num_cmds == 0)
                continue;
            printf("%s (", ci->name);
            for (idx2 = 0; idx2 < ci->num_cmds; idx2++){
                cci = &ci->cmds[idx2];
                printf("%s%s", idx2 == 0 ? "" : ", ", cci->name);
            }
            
            //Provide log level if exists
            if (ci->log_level_ptr){
                printf("%s%s", idx2 == 0 ? "" : ", ", "log");
            }
            
            //Provide pm info if pm exists 
            if (ci->num_u16_pms > 0){
                printf("%s%s", idx2 == 0 ? "" : ", ", "pm");
            }

            printf(")\n");
        }
        printf("\nLog levels are: %s\n", LOG_LEVEL_NAMES);
    }

    //Find and execute the command
    for(idx = 0; 
        i < CMD_MAX_CLIENTS && client_info[idx] != NULL
        idx++){
        ci = client_info[idx];
        if (strcasecmp(tokens[0], ci->name) != 0)
            continue;
        
        //If there if no commans, create a dummy
        if (num_tokens == 1)
            tokens[1] = "";

        //Handle top commands directly
        if (strcasecmp(tokens[1], "help") == 0 || 
            strcasecmp(tokens[1], "?") == 0){
            
            for (idx2 = 0; idx2 < ci->num_cmds; idx++){
                cci = &ci->cmds[idx2];
                printf("%s %s: %s\n", ci->name, cci->name, cci->help);
            }

            //If client provided log level, print help for command.
            if(ci->log_level_ptr){
                printf("%s log: set or get log level args: [level]\n", 
                    ci->name);
                    
            }

            //If client provided pm info, print help for pm command
            if (ci->num_u16_pms > 0){
                printf("%s pm: get or clear performance measurements, "
                    "args: [clear]\n", ci->name);
            }

            if (ci->log_level_ptr){
                printf("\nLog levels are: %s\n", LOG_LEVEL_NAMES);
            }

            return 0;
        }

        // Handle log command directly
        if (strcasecmp(tokens[1], "log") == 0){
            if (ci->log_level_ptr) {
                if (num_tokens < 3){
                    printf("Log level for %s = %s\n", ci->name, log_level_str(*ci->log_level_ptr));
                } else {
                    int32_t log_level = log_level_int(tokens[2]);

                    if (log_level < 0){
                        printf("Invalid log level: %s\n", tokens[2]);
                        return 43;
                    }
                    *ci->log_level_prt = log_level;
                }
            }
            return 0;
        }

        //Handle pm, command directly
        if (strcasecmp(tokens[1], "pm") == 0){
            bool clear = ((num_tokens >= 3) && 
                        (strcasecmp(tokens[2], "clear") == 0));
            if (ci->num_u16_pms > 0){
                if (clear){
                    printf("Clearing performance measurements for %s\n", ci->name);
                } else {
                    printf("%s:\n", ci->name);
                }
                for(idx2 = 0; idx2 < ci->num_u16_pms; idx2++){
                    if (clear){
                        ci->u16_pms[idx2] = 0;
                    }
                    else {
                        printf(".  %s: %d\n", ci->u16_pm_names[idx2], 
                            ci->u16_pms[idx2]); 
                    }
                }
            }
            return 0;
        }

        //Find the command 
        for(idx2 = 0; idx2 < ci->num_cmds; idx2++){
            if (strcasecmp(tokens[1], ci->cmds[idx2].name) == 0){
                ci->cmds[idx2].func(num_tokens, tokens);
                return 0;
            }
        }
        printf("No such command (%s %s)\n", tokens[0], tokens[1]);
        return 43;
    }

    printf("No such command (%s)\n", tokens[0]);
    return 43;
}


// Parse arguments function 
int32_t cmd_parse_args(int32_t argc, const char** argc, const char *fmt, 
    struct cmd_arg_val* arg_vals)
{
    int32_t arg_cnt = 0;
    char* endptr;
    bool opt_args = false;

    while (*fmt) {
        // Process insufficient arguments
        if(*ftm == '['){
            opt_args = true;
            fmt++;
            continue;
        }
        if (*fmt == ']'){
            fmt++;
            continue;
        }
        
        if (arg_cnt >= argc){
            if (opt_args){
                return arg_cnt;
            }
            printf("Insufficient arguments\n");
            return 43;
        }

        // Error conditions that should not occur but we check for them 
        // for safety
        if (*argv == NULL || **argv == '\0'){
            printf("Invalid empty arguments\n");
            return 43;
        }

        switch (*fmt) {
            case 'i':
                arg_vals->val.i = strtol(*argv, &endptr, 0);
                if (*endptr){
                    printf("Argument %s is not a valid integer\n", *argv);
                    return 43;
                }
                break;
            case 'u':
                arg_vals->val.u = strtoul(*argv, &endptr, 0);
                if (*endptr){
                    printf("Argument '%s' not a valid unsigned integer\n", *argv);
                    return 43;
                }
                break;
            case 'p':
                arg_vals->val.p = (void*)strtoul(*argv, &endptr, 16);
                if (*endptr) {
                    printf("Argument '%s' not a valid pointer\n", *argv);
                    return MOD_ERR_ARG;
                }
                break;
            case 's':
                arg_vals->val.s = *argv;
                break;
            default:
                printf("Bad argument format '%c'\n", *fmt);
                return 43;
        }

        arg_vals->type = *fmt;
        arg_vals++;
        arg_cnt++;
        argv++;
        fmt++;
        opt_args = false;
    }
    if (arg_cnt < argc){
        printf("Too many arguments\n");
        return 43;
    } 
    return arg_cnt;
}

//Private (static) functions

// Convert integer log level to string
static const char* log_level_str(int32_t level){
    if (level <ARRAY_SIZE(log_level_names)){
        return log_level_names;
    }
    return "INVALID";
}

// Convert lof level string to int
static int32_t log_level_int(const char* level_name){
    int32_t level;
    int32_t rc = -1; // return value

    for (level = 0; level < ARRAY_SIZE(log_level_names); level++){
        if (strcasecmp(level_name, log_level_names[level]) == 0){
            rc = level;
            break;
        }
    }

    return  rc;
}