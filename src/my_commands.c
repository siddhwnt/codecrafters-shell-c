#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

#include "my_commands.h"
#include "main.h"

char program_dir[4096];

Command my_commands[] = {
    {"pwd", handle_pwd},
    {"cd", handle_cd}};

#define MY_COMMANDS_COUNT (sizeof(my_commands) / sizeof(my_commands[0]))

bool isDirectoryExists(const char *path)
{
    struct stat sb;

    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
    {
        return true;
    }

    return false;
}

char *handle_pwd(char *args[])
{
    char *output = malloc(4096);

    if (output == NULL)
    {
        perror("malloc");
        return NULL;
    }

    strcpy(output, program_dir);

    strcat(output, "\n");
    return output;
}

char *handle_cd(char *args[])
{
    int arg_count = 0;

    while (args[arg_count] != NULL)
        arg_count++;

    if (arg_count > 1)
    {
        fprintf(stderr, "cd: too many arguments\n");
        return NULL;
    }

    char *directory;

    if (arg_count == 0)
        directory = getenv("HOME");
    else
        directory = args[0];

    if (strcmp(directory, "~") == 0)
        directory = getenv("HOME");

    if (chdir(directory) != 0)
    {
        char *error = malloc(4096);

        if (error == NULL)
        {
            perror("malloc");
            return NULL;
        }

        fprintf(stderr, "cd: %s: No such file or directory\n", directory);
        return NULL;
    }

    getcwd(program_dir, sizeof(program_dir));

    return NULL;
}
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

char *handleCommand(char *command, char *args[])
{
    for (int i = 0; i < MY_COMMANDS_COUNT; i++)
    {
        if (strcmp(my_commands[i].name, command) == 0)
        {
            return my_commands[i].handler(args);
        }
    }

    return NULL;
}