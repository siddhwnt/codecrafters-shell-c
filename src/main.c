#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

const char *path;

char *builtin_words[] = {
    "echo",
    "type",
    "exit"};

int builtin_words_count = sizeof(builtin_words) / sizeof(builtin_words[0]);

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
      printf("%s is a shell builtin\n", rest);
      return true;
    }
  }
  // printf("%s: not found\n", rest);
  return false;
}

bool isInPath(char *rest)
{
  // printf("Checking in PATH\n");
  int rest_size = strlen(rest);
  char *path_locations[100];
  char *context = NULL;

  path = getenv("PATH");
  int path_size = strlen(path);

  // printf("PATH = %s\n", path);
  char path_copy[4096];

  strcpy(path_copy, path);

  // Seperate different paths in PATH
  char *token = strtok(path_copy, ":");

  int i = 0;
  while (token != NULL)
  {
    path_locations[i] = token;
    token = strtok(NULL, ":");
    // printf("%s\n", path_locations[i]);
    i++;
  }

  for (int j = 0; j < i; j++)
  {
    // Get exact location
    char rest_location[4096];
    strcpy(rest_location, path_locations[j]);
    strcat(rest_location, "/");
    strcat(rest_location, rest);


    int current_size = strlen(path_locations[j]);
    // printf("Path: %s\n", path_locations[j]);
    // printf("Checking : %s\n", rest_location);

    if (isFileExists(rest_location))
    {
      printf("%s is %s\n", rest, rest_location);
      return true;
    }
  }
  return false;
}

void parseCommand(char *command, char *rest)
{

  // Handle echo
  if (strcmp(command, "echo") == 0)
  {
    printf("%s\n", rest);
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
      return;
    }
    else if (isInPath(rest))
    {
      return;
    }
    else
    {
      printf("%s: not found\n", rest);
      return;
    }
  }
  printf("%s: command not found\n", command);
}

void parseText(char *text)
{
  int command_size = 0;
  char command[20];

  while (text[command_size] != ' ' && text[command_size] != '\0')
  {
    command_size++;
  }

  memcpy(command, text, command_size);
  command[command_size] = '\0';

  char *rest = text + command_size + 1;

  while (*rest == ' ')
  {
    rest++;
  }

  parseCommand(command, rest);
}

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  // TODO: Uncomment the code below to pass the first stage

  char text[100];

  while (1)
  {
    printf("$ ");
    fgets(text, sizeof(text), stdin);
    text[strcspn(text, "\n")] = '\0';
    parseText(text);
  }

  return 0;
}
