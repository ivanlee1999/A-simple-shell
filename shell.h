#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// #include <error.h>

#define MAXARG 128
#define MAXLINE 1024
char env[100][100] = {"/bin"};
char error_message[30] = "An error has occurred\n";
// write(STDERR_FILENO, error_message, strlen(error_message)); 
// char* env[] = {"/usr/bin/"};
int envNum = 2;
char prompt[] = "wish> ";
int numOfCommand = 0;
int redirect = 0;


struct cmdWithArg{
    char* cmd[MAXARG];
    int ifRederect;
    char* redirectFile;
    int numberOfCommand;
    int notRun;
};

int runCommand(struct cmdWithArg *cmdLine);


int parse(const char *cmdLine, struct cmdWithArg *parsedCmd);

int exit2();

int cd(char* dir);

int path(char**);

int if2(struct cmdWithArg *cmdLine);