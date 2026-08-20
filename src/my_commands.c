#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "my_commands.h"

void handle_pwd(char *args[])
{
    printf("Handle pwd here\n");
}

Command my_commands[] = {
    {"pwd", handle_pwd},
};

#define MY_COMMANDS_COUNT (sizeof(my_commands) / sizeof(my_commands[0]))

bool find_command(char *command)
{
    for (int i = 0; i < MY_COMMANDS_COUNT; i++)
    {
        if (strcmp(my_commands[i].name, command) == 0)
        {            
            return true;
        }
    }
    return false;
}

void handleCommand(char *command, char *rest)
{
    char *args[100];
    int argc = 0;

    args[argc++] = command;

    char *token = strtok(rest, " ");

    while (token != NULL && argc < 99)
    {
        args[argc++] = token;
        token = strtok(NULL, " ");
    }

    args[argc] = NULL;

    for (int i = 0; i < MY_COMMANDS_COUNT; i++)
    {
        if (strcmp(my_commands[i].name, command) == 0)
        {
            my_commands[i].handler(args);
            return;
        }
    }
}