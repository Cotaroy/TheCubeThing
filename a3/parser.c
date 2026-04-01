#include "parser.h"
#include "manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_COMMAND_ARGUMENT_COUNT 8

static void end_string_before_line_feed(char *str) {
    char *line_feed_position = strchr(str, '\n');
    if(line_feed_position != NULL) {
        *line_feed_position = '\0';
    }
}

int parse_single_command() {
    char command[MAX_COMMAND_LENGTH];

    // "'"'"'"'"temporary"'"'"'"'"  fix for the problem
    // where when you pipe in something from a file
    // and stdin gets changed back to your terminal after the file reaches EOF,
    // the first command you type does not get acknowledged
    // because of a straggling line feed character in the stdin maybe???
    // this problem caused `fgets` to read one line and get nothing
    // causing the command handler to not get fired.
    // this loop just tries again if an empty line is somehow read
    while(1) {
        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
            perror("fgets");
            exit(1);
        }

        // separate out the arguments of the command, delimited by spaces
        char *argv[MAX_COMMAND_ARGUMENT_COUNT];
        argv[0] = strtok(command, " ");
        end_string_before_line_feed(argv[0]);
        int argc = 1;

        if(strlen(argv[0]) == 0) {
            continue;
        }

        while(argc < MAX_COMMAND_ARGUMENT_COUNT) {
            argv[argc] = strtok(NULL, " ");
            if(argv[argc] == NULL) {
                break;
            }
            end_string_before_line_feed(argv[argc]);
            argc++;
        }

        CommandHandler *handler = get_command_handler(argv[0]);
        if (handler == NULL) {
            return -1;
        }

        return handler->handle_command(argc, argv);
    }

    return -1;
}

void parse_at_startup() {}
