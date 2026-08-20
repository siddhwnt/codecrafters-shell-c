typedef void (*CommandHandler)(char *args[]);

typedef struct
{
    char *name;
    CommandHandler handler;
} Command;

void handle_pwd(char *args[]);
void handle_cd(char *args[]);
bool find_command(char *command);

void handleCommand(char *command, char *rest);