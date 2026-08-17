#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* builtin_words[] = {
  "echo",
  "type",
  "exit"
};

int builtin_words_count = sizeof(builtin_words) / sizeof(builtin_words[0]);

void parseCommand(char* command, char* rest)
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
      for(int i = 0; i < builtin_words_count; i++)
      {
        if(strcmp(builtin_words[i], rest)==0)
        {
          printf("%s is a shell builtin\n", rest);
          return;
        }
      }
      printf("%s: not found\n", rest);
      return;
    }
    printf("%s: command not found\n", command);
}

void parseText(char* text)
{
    int command_size = 0;
    char command[20];
    
    while (text[command_size] != ' ' && text[command_size] != '\0')
    {
      command_size++;
    }
    
    memcpy(command, text, command_size);
    command[command_size] = '\0';

    char* rest = text + command_size + 1;

    while(* rest == ' ')
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
