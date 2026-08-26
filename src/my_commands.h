extern char program_dir[4096];

typedef void (*CommandHandler)(char *args[]);

typedef struct
{
    char *name;
    CommandHandler handler;
} Command;

void handle_pwd(char *args[]);
void handle_cd(char *args[]);
bool find_command(char *command);

char *handleCommand(char *command, char *args[]);