#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#include "my_commands.h"
#include "main.h"
#include <fcntl.h>

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

// File redirection
bool isFileRedirected = false;
char *redirect_file = NULL;
static char redirect_storage[4096];
#define OUTPUT_SIZE 4096

void setUpPath()
{
  path = getenv("PATH");

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
  current_path_count = 0;

  for (int j = 0; j < path_count; j++)
  {
    // Get exact location
    char rest_location[4096];
    strcpy(rest_location, path_locations[j]);
    strcat(rest_location, "/");
    strcat(rest_location, rest);

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
char *handleEcho(char *args[])
{
  char *output = malloc(OUTPUT_SIZE);

  if (output == NULL)
  {
    perror("malloc");
    return NULL;
  }

  output[0] = '\0';

  int i = 0;

  while (args[i] != NULL)
  {
    strcat(output, args[i]);

    if (args[i + 1] != NULL)
      strcat(output, " ");

    i++;
  }
  strcat(output, "\n");
  return output;
}

char *handlePathExecutable(char *command, char *args[])
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

  int pipe_fd[2];
  // fd[0] -> read
  // fd[1] -> write

  if (pipe(pipe_fd) == -1)
  {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  int command_id = fork();

  if (command_id == -1)
  {
    perror("fork");
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    return NULL;
  }

  if (command_id == 0)
  {
    // Child
    // Process will only write
    close(pipe_fd[0]);
    if (dup2(pipe_fd[1], STDOUT_FILENO) == -1)
    {
      perror("dup2");
      exit(EXIT_FAILURE);
    }

    close(pipe_fd[1]);

    execv(command_location, argv);

    perror("execv");
    exit(EXIT_FAILURE);
  }

  // Parent
  // Process will read
  close(pipe_fd[1]);

  ssize_t total = 0;
  ssize_t bytes_read;
  char *output = malloc(OUTPUT_SIZE);

  if (output == NULL)
  {
    perror("malloc");
    close(pipe_fd[0]);
    waitpid(command_id, NULL, 0);
    return NULL;
  }

  while ((total < OUTPUT_SIZE - 1) &&
         (bytes_read = read(pipe_fd[0],
                            output + total,
                            OUTPUT_SIZE - 1 - total)) > 0)
  {
    total += bytes_read;
  }

  output[total] = '\0';
  close(pipe_fd[0]);
  waitpid(command_id, NULL, 0);

  return output;
}

void parseRest(char *rest, char *args[])
{
  static char storage[100][4096];
  for (int i = 0; i < 100; i++)
  {
    args[i] = storage[i];
    storage[i][0] = '\0';
  }

  bool inside_single_quotes = false;
  bool inside_double_quotes = false;
  int args_count = 0;
  int arg_pos = 0;

  for (int i = 0; rest[i] != '\0'; i++)
  {
    char c = rest[i];

    // Inside single quotes
    if (inside_single_quotes)
    {
      if (c == '\'')
        inside_single_quotes = false;

      else
        args[args_count][arg_pos++] = c;

      continue;
    }

    // Inside double quotes
    if (inside_double_quotes)
    {
      if (c == '"')
        inside_double_quotes = false;
      else if (c == '\\')
      {
        if (rest[i + 1] == '\0')
        {
          printf("\\ cannot be at the end\n");
          return;
        }
        else if (rest[i + 1] == '\"' || rest[i + 1] == '\\')
        {
          // handle ", \, $, `, and newline after "\"
          args[args_count][arg_pos++] = rest[++i];
        }
        else
        {
          // handle other special characters after "\"
        }
      }
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

    else if (c == '>' || c == '1')
    {
      isFileRedirected = true;
      if (c == '1')
      {
        int j = i + 1;

        // Allow spaces between 1 and >
        while (rest[j] == ' ')
          j++;

        if (rest[j] != '>')
        {
          args[args_count][arg_pos++] = c;
          continue;
        }

        i = j;
      }

      // rest[i] = '>' currently, so skip
      i++;

      while (rest[i] == ' ')
        i++;

      int redirect_pos = 0;
      bool single = false;
      bool double_quote = false;

      while (rest[i] != '\0')
      {
        c = rest[i];

        if (single)
        {
          if (c == '\'')
            single = false;
          else
            redirect_storage[redirect_pos++] = c;
        }
        else if (double_quote)
        {
          if (c == '"')
            double_quote = false;
          else
            redirect_storage[redirect_pos++] = c;
        }
        else
        {
          if (c == '\'')
            single = true;
          else if (c == '"')
            double_quote = true;
          else
            redirect_storage[redirect_pos++] = c;
        }

        i++;
      }

      redirect_storage[redirect_pos] = '\0';
      redirect_file = redirect_storage;

      break;
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

char *handleType(char *args[])
{
  char *output = malloc(4096);

  if (output == NULL)
  {
    perror("malloc");
    return NULL;
  }

  output[0] = '\0';

  int i = 0;

  while (args[i] != NULL)
  {
    char *keyword = args[i];

    if (isBuiltIn(keyword))
    {
      strcat(output, keyword);
      strcat(output, " is a shell builtin\n");
    }
    else if (isInPath(keyword))
    {
      char keyword_location[4096];

      strcpy(keyword_location, path_locations[current_path_count]);
      strcat(keyword_location, "/");
      strcat(keyword_location, keyword);

      strcat(output, keyword);
      strcat(output, " is ");
      strcat(output, keyword_location);
      strcat(output, "\n");
    }
    else
    {
      strcat(output, keyword);
      strcat(output, ": not found\n");
    }

    i++;
  }

  return output;
}

char *executeCommand(char *command, char *args[])
{
  // Handle echo
  if (strcmp(command, "echo") == 0)
  {
    return handleEcho(args);
  }

  // Handle exit
  if (strcmp(command, "exit") == 0)
  {
    exit(EXIT_SUCCESS);
  }

  // Handle type
  if (strcmp(command, "type") == 0)
  {
    return handleType(args);
  }

  // Handle command in PATH
  if (isInPath(command))
  {
    return handlePathExecutable(command, args);
  }

  // Handle custom commands
  if (find_command(command))
  {
    return handleCommand(command, args);
  }

  printf("%s: command not found\n", command);
  return NULL;
}

int getCommand(char *text, char *command)
{
  bool inside_single_quotes = false, inside_double_quotes = false;
  int arg_pos = 0;
  for (int i = 0; i < strlen(text); i++)
  {
    char c = text[i];

    // Inside single quotes
    if (inside_single_quotes)
    {
      if (c == '\'')
        inside_single_quotes = false;

      else
        command[arg_pos++] = c;

      continue;
    }

    // Inside double quotes
    if (inside_double_quotes)
    {
      if (c == '"')
        inside_double_quotes = false;
      else if (c == '\\')
      {
        if (text[i + 1] == '\0')
        {
          printf("\\ cannot be at the end\n");
          return -1;
        }
        else if (text[i + 1] == '\"' || text[i + 1] == '\\')
        {
          // handle ", \, $, `, and newline after "\"
          command[arg_pos++] = text[++i];
        }
        else
        {
          // handle other special characters after "\"
        }
      }
      else
        command[arg_pos++] = c;

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
      command[arg_pos] = '\0';
      return i;
    }
    else if (c == '\\')
    {
      if (text[i + 1] == '\0')
      {
        printf("\\ cannot be at the end\n");
        return -1;
      }

      command[arg_pos++] = text[++i];
    }
    else
    {
      command[arg_pos++] = c;
    }
  }
  command[arg_pos] = '\0';
  return strlen(text);
}

void parseText(char *text, char *command, char *args[])
{
  isFileRedirected = false;
  redirect_file = NULL;
  redirect_storage[0] = '\0';

  int command_size = getCommand(text, command);
  if (command_size == -1)
  {
    printf("Something went wrong in command parsing\n");
    return;
  }

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

  parseRest(rest, args);
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

    char command[100];
    char *args[100];

    parseText(text, command, args);

    char *output = executeCommand(command, args);

    if (output != NULL)
    {
      if (redirect_file != NULL)
      {
        FILE *fptr_redirected = fopen(redirect_file, "w");

        if (fptr_redirected == NULL)
        {
          perror("fopen");
        }
        else
        {
          fprintf(fptr_redirected, "%s", output);
          fclose(fptr_redirected);
        }
      }
      else
      {
        printf("%s", output);
      }
      
      free(output);
    }
  }

  return 0;
}
