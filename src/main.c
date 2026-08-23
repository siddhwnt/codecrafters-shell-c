#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#include "my_commands.h"
#include "main.h"

const char *builtin_words[] = {
    "echo",
    "type",
    "exit",
    "pwd",
    "cd"};

const size_t builtin_words_count = sizeof(builtin_words) / sizeof(builtin_words[0]);

static char path_copy[4096];

// Global variables for PATH
const char *path;
char *path_locations[100];
int path_count = 0;

// current_path_count is for getting the location
// of the command being used in isInPath() function
int current_path_count;

void setUpPath()
{
  path = getenv("PATH");
  int path_size = strlen(path);

  strcpy(path_copy, path);

  // Seperate different paths in PATH
  char *token = strtok(path_copy, ":");

  while (token != NULL)
  {
    path_locations[path_count++] = token;
    token = strtok(NULL, ":");
  }
}

int isFileExists(const char *path)
{
  // Try to open file
  FILE *fptr = fopen(path, "r");

  // If file does not exists
  if (fptr == NULL)
    return 0;

  // File exists hence close file and return true.
  fclose(fptr);

  return 1;
}

bool isBuiltIn(char *rest)
{
  // printf("Checking in builtin\n");
  for (int i = 0; i < builtin_words_count; i++)
  {
    if (strcmp(builtin_words[i], rest) == 0)
    {
      return true;
    }
  }
  return false;
}

bool isExecutable(char *filepath)
{
  if (access(filepath, X_OK) == 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool isInPath(char *rest)
{
  // printf("Checking in PATH\n");
  int rest_size = strlen(rest);
  current_path_count = 0;

  for (int j = 0; j < path_count; j++)
  {
    // Get exact location
    char rest_location[4096];
    strcpy(rest_location, path_locations[j]);
    strcat(rest_location, "/");
    strcat(rest_location, rest);

    int current_size = strlen(path_locations[j]);
    // printf("Path: %s\n", path_locations[j]);
    // printf("Checking : %s\n", rest_location);

    if (isExecutable(rest_location))
    {
      current_path_count = j;
      return true;
    }
  }
  return false;
}

// Handlers
void handleEcho(char *args[])
{
  int i = 0;

  while (args[i] != NULL)
  {
    printf("%s", args[i]);

    if (args[i + 1] != NULL)
    {
      printf(" ");
    }

    i++;
  }

  printf("\n");
}

void handlePathExecutable(char *command, char *args[])
{
  char command_location[4096];

  strcpy(command_location, path_locations[current_path_count]);
  strcat(command_location, "/");
  strcat(command_location, command);

  char *argv[101];

  argv[0] = command;

  int i = 0;

  while (args[i] != NULL)
  {
    argv[i + 1] = args[i];
    i++;
  }

  argv[i + 1] = NULL;

  int command_id = fork();

  if (command_id == 0)
  {
    execv(command_location, argv);

    perror("execv");
    exit(EXIT_FAILURE);
  }
  else
  {
    wait(NULL);
  }
}

void parseRest(char *rest, char *args[])
{
  static char storage[100][4096];

  bool inside_single_quotes = false;
  bool inside_double_quotes = false;
  int args_count = 0;
  int arg_pos = 0;

  for (int i = 0; i < 100; i++)
  {
    args[i] = storage[i];
    storage[i][0] = '\0';
  }

  for (int i = 0; rest[i] != '\0'; i++)
  {
    char c = rest[i];

    // Inside single quotes
    if (inside_single_quotes)
    {
      if (c == '\'')
        inside_single_quotes = false;

      // else if (c == '\\')
      // {
      //   if (rest[i + 1] == '\0')
      //   {
      //     printf("\\ cannot be at the end\n");
      //     return;
      //   }

      //   args[args_count][arg_pos++] = rest[++i];
      // }
      else
        args[args_count][arg_pos++] = c;

      continue;
    }

    // Inside double quotes
    if (inside_double_quotes)
    {
      if (c == '"')
        inside_double_quotes = false;
      else
        args[args_count][arg_pos++] = c;

      continue;
    }

    // Outside quotes
    if (c == '\'')
    {
      inside_single_quotes = true;
    }
    else if (c == '"')
    {
      inside_double_quotes = true;
    }
    else if (c == ' ')
    {
      if (arg_pos > 0)
      {
        args[args_count][arg_pos] = '\0';
        args_count++;
        arg_pos = 0;
      }
    }
    else if (c == '\\')
    {
      if (rest[i + 1] == '\0')
      {
        printf("\\ cannot be at the end\n");
        return;
      }

      args[args_count][arg_pos++] = rest[++i];
    }
    else
    {
      args[args_count][arg_pos++] = c;
    }
  }

  if (arg_pos > 0)
  {
    args[args_count][arg_pos] = '\0';
    args_count++;
  }

  args[args_count] = NULL;
}
void parseCommand(char *command, char *rest)
{
  // Parse rest here to handle quotes
  char *args[100];
  parseRest(rest, args);

  // Handle echo
  if (strcmp(command, "echo") == 0)
  {
    handleEcho(args);
    return;
  }

  // Handle exit
  if (strcmp(command, "exit") == 0)
  {
    exit(EXIT_SUCCESS);
  }

  // Handle type
  if (strcmp(command, "type") == 0)
  {
    // Check if builtin word
    if (isBuiltIn(rest))
    {
      printf("%s is a shell builtin\n", rest);
      return;
    }
    else if (isInPath(rest))
    {
      char rest_location[4096];
      strcpy(rest_location, path_locations[current_path_count]);
      strcat(rest_location, "/");
      strcat(rest_location, rest);
      printf("%s is %s\n", rest, rest_location);
      return;
    }
    else
    {
      printf("%s: not found\n", rest);
      return;
    }
  }

  // Handle if command in PATH
  if (isInPath(command))
  {
    handlePathExecutable(command, args);
    return;
  }

  // Check if custom command (implemented in my_commands.c)
  if (find_command(command))
  {
    handleCommand(command, rest);
    return;
  }

  printf("%s: command not found\n", command);
}

void parseText(char *text)
{
  int command_size = 0;
  char command[100];

  while (text[command_size] != ' ' && text[command_size] != '\0')
  {
    command_size++;
  }

  memcpy(command, text, command_size);
  command[command_size] = '\0';

  char *rest;

  if (text[command_size] == '\0')
  {
    rest = text + command_size;
  }
  else
  {
    rest = text + command_size + 1;

    while (*rest == ' ')
    {
      rest++;
    }
  }

  parseCommand(command, rest);
}

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  setUpPath();

  char text[4096];

  // Setup current directory

  if (getcwd(program_dir, sizeof(program_dir)) == NULL)
  {
    printf("Cannot initialize current directory");
  }

  // Start shell loop
  while (1)
  {
    printf("$ ");
    if (fgets(text, sizeof(text), stdin) == NULL)
    {
      break;
    }
    text[strcspn(text, "\n")] = '\0';
    parseText(text);
  }

  return 0;
}
