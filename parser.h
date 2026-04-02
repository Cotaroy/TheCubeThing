#ifndef PARSER_H
#define PARSER_H

#define NUM_AVAILABLE_COMMAND_HANDLERS 8

// read from standard in and execture commands accordingly
int parse_single_command();

void parse_at_startup();

/**
 * Is considered to be a null-handler if its name is null.
 */
typedef struct {
    char *command_name;
    int (*handle_command)(int argc, char **argv);
} CommandHandler;

CommandHandler *get_command_handler(char *command_name);

#endif
