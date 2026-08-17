#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  // TODO: Uncomment the code below to pass the first stage

  char command[30];

  while (1)
  {
    printf("$ ");
    scanf("%s", command);

    if (strcmp(command, "exit") == 0)
      break;

    printf("%s: command not found\n", command);
  }

  return 0;
}
