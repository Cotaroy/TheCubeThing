#include <stdio.h>
#include <stdlib.h>
#include "manager.h"

#define MAX_COMMAND_NAME_SIZE 15

void parse_single_command() {
    char command_name[MAX_COMMAND_NAME_SIZE];
    int scanf_res = scanf("%s", command_name);

    if (scanf_res == -1 && ferror(stdin)) {
        perror("scanf");
        exit(1);
    }
}

void parse_at_startup() {

}
