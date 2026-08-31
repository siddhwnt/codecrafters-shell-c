extern char program_dir[4096];

typedef char* (*CommandHandler)(char *args[]);

typedef struct
{
    char *name;
    CommandHandler handler;
} Command;

char* handle_pwd(char *args[]);
char* handle_cd(char *args[]);
bool find_command(char *command);

char *handleCommand(char *command, char *args[]);