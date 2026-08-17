#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  // TODO: Uncomment the code below to pass the first stage

  char text[100];
  char command[20];

  while (1)
  {
    printf("$ ");
    fgets(text, sizeof(text), stdin);
    int i = 0;

    while (text[i] != ' ' && text[i] != '\n' && text[i] != '\0')
    {
      i++;
    }

    memcpy(command, text, i);
    command[i] = '\0';

    if (strcmp(command, "echo") == 0)
    {
      printf("%s\n", text + i + 1);
      continue;
    }

    if (strcmp(command, "exit") == 0)
      break;

    printf("%s: command not found\n", command);
  }

  return 0;
}
