#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

#include "my_commands.h"
#include "main.h"

char program_dir[4096];

bool isDirectoryExists(const char *path)
{
    struct stat sb;
    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
    {
        return true; // Directory exists
    }
    return false; // Does not exist or is not a directory
}

void handle_pwd(char *args[])
{
    printf("%s\n", program_dir);
}

void handle_cd(char *args[])
{
    int arg_count = 0;

    while (args[arg_count] != NULL)
    {
        arg_count++;
    }

    if (arg_count > 2)
    {
        printf("cd: too many arguments\n");
        return;
    }

    char *directory;

    // cd with no argument
    if (arg_count == 1)
    {
        directory = program_dir;
    }
    else
    {
        directory = args[1];
    }

    // cd ~
    if (strcmp(directory, "~") == 0)
    {
        directory = getenv("HOME");
    }

    if (chdir(directory) != 0)
    {
        printf("cd: %s: No such file or directory\n", directory);
        return;
    }

    getcwd(program_dir, sizeof(program_dir));
}


Command my_commands[] = {
    {"pwd", handle_pwd},
    {"cd", handle_cd}};

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