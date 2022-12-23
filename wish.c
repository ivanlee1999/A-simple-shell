#include "shell.h"

int main(int argc, char **argv)
{

    //batch mode
    char cmdLine[MAXLINE];
    int isBuildIn = 1;
    // printf("input number %d input : %s \n", argc, argv[1]);
    // printf("argc number %d  \n", argc);
    if(argc > 2){
        // printf("muliple batch file\n");
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        return 1;
    }
    // printf("in batch mode/n");
    if(argc == 2){
        // printf("in bach mode\n");
        FILE * fp;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        char* fileName = argv[1];
        // char* fileName = "batch.txt";
        fp = fopen(fileName, "r");
        if (fp == NULL){
            // printf("unable to open file");
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
            // exit(EXIT_FAILURE);
        }
        int i = 0;
        while ((read = getline(&line, &len, fp)) != -1) {
            // if (strlen(line) == 1) {
            //     printf("H2CO3 spotted a blank line\n");
            // }

            // printf("line length %ld \n", strlen(line));

            if(strlen(line) > 1){
                numOfCommand = 0;
            // printf("Retrieved line of length %zu:\n", read);

            // line[strlen(line) - 1] = '\0';
                struct cmdWithArg *parsedCmd = malloc(sizeof(struct cmdWithArg));
                parse(line, parsedCmd);
                // printf("start running\n");
                // int commandReturn = 
                runCommand(parsedCmd);
                // printf("%d th line %s   %ld \n", i, line, strlen(line));
                i++;
            // printf("return value %d \n", commandReturn);
            }
        }
        // exit(0);

        fclose(fp);
        return 0;
    }

    // printf("interactive mode\n");
// interactive mode
    while(1){
        numOfCommand = 0;
        printf("%s", prompt);
        fgets(cmdLine, MAXLINE, stdin);
        if(feof(stdin)){
            printf("\n");
            exit(0);
        }
        
        cmdLine[strlen(cmdLine) - 1] = '\0';
        
        struct cmdWithArg *parsedCmd = malloc(sizeof(struct cmdWithArg));
        
        parsedCmd->ifRederect = 0;

        parse(cmdLine, parsedCmd);
        int commandReturn = runCommand(parsedCmd);
        // printf("%d \n", commandReturn);
    }

    return 0 ;
};

int runCommand(struct cmdWithArg *parsedCmd){
    // printf("start running command %s\n", parsedCmd->cmd[0]);
    if(parsedCmd->notRun == 1){
        return 0;
    }
    if(strcmp(parsedCmd->cmd[0], "exit") == 0){
        if(parsedCmd->cmd[1] != NULL){
            write(STDERR_FILENO, error_message, strlen(error_message)); 
        }
        // printf("start existing\n");
        exit(0);
        return 1;
    }
    else if(strcmp(parsedCmd->cmd[0], "cd") == 0){
        return cd(parsedCmd->cmd[1]);
    }
    else if(strcmp(parsedCmd->cmd[0], "path") == 0){
        path(parsedCmd->cmd);
        // printf("env number is %d \n", envNum);
        return 1;
    }
    else if(strcmp(parsedCmd->cmd[0], "if") == 0){
        // printf("this is an if command\n");
        return if2(parsedCmd);
    }
    // without redirection
    else if(parsedCmd->ifRederect == 0){
    //    printf("no redirecting\n"); 
        pid_t childPid = fork();
        if(childPid < 0){
            // error("there is an error during fork");
        }
        else if(childPid == 0) {       // this is child process
            // char* path = env;
            for(int i = 0; i< envNum; i++){

                char path[100];
                strcat(env[i], "/");
                strcpy(path, env[i]);
                strcat(path, parsedCmd->cmd[0]);
                // printf("path: %s \n", path);
                // printf("system command: %s \n", path);
                // printf("command name %s\n", parsedCmd->cmd[0]);
                if(execv(path, parsedCmd->cmd) == -1){
                    // printf("%s: This command doesn't exist\n", parsedCmd->cmd[0]);
                }
                                    
            }
             write(STDERR_FILENO, error_message, strlen(error_message));
             exit(0);
            // exit(0);
            // char *params[4]  = {cmdLine};
        }
        else{
            int status;
            waitpid(childPid, &status, 0);
            // printf("status %d \n",  WEXITSTATUS(status));
            return  WEXITSTATUS(status);
        }
    }
    // with redirection
    else if(parsedCmd->ifRederect == 1){
        // if(parsedCmd->numberOfCommand == 1){
        //     write(STDERR_FILENO, error_message, strlen(error_message)); 
        //     return 0;
        // }
        // printf("start redirecting\n");
        int link[2];
        char foo[4096];

        pipe(link);

        pid_t childPid = fork();
        if(childPid < 0){
            // error("there is an error during fork");
        }
        else if(childPid == 0) {       // this is child process
            // char* path = env;
            for(int i = 0; i< envNum; i++){
                
                // dup2 (link[1], 1);
                // close(link[0]);
                // close(link[1]);
                close(link[0]);
                dup2(link[1], 1);
                close(link[1]);

                char path[100];
                strcat(env[i], "/");
                strcpy(path, env[i]);
                strcat(path, parsedCmd->cmd[0]);
                // printf("path: %s \n", env[i]);
                // printf("system command: %s \n", path);
                if(execv(path, parsedCmd->cmd) == -1){
                    // printf("%s: This command doesn't exist\n", cmdLine[0]);
                    // exit(0);
                }   
            }
            write(STDERR_FILENO, error_message, strlen(error_message));
            // exit(0);
        }
        else{
            int status;
            waitpid(childPid, &status, 0);

            close(link[1]);
            int nbytes = read(link[0], foo, sizeof(foo));
            // for(int i = 0; i <sizeof(foo); i++){
            //     if(foo[i] == '\n'){
            //         foo[i] = ' ';
            //     }
            // }

            // write output to destination
            FILE *out;
            out = fopen(parsedCmd->redirectFile,"w+");
            if ( out ){
                // fputs(foo,fp);
                fwrite(foo, nbytes, 1, out);
            }
            else{
                // printf("Failed to open the file\n");
            }
            fclose(out);
            // printf("Output: %.*s \n", nbytes, foo);
            // printf("Output: (%.*s)\n", nbytes, foo);
            return  WEXITSTATUS(status);
        }
    }
}







int parse(const char *cmdLine, struct cmdWithArg *parsedCmd){
    // printf("start parsing %s\n", cmdLine);
    char array[MAXLINE];
    char* startOfCommand = array;
    char* endOfCommand;
    char* realEndOfCommand;
    char special[10] = " \t\r\n";
    char* skip;
    int fileNameStartPosition = 0;
    parsedCmd->ifRederect = 0;
    parsedCmd->notRun = 0;
    // numOfCommand = 0;

    // if(cmdLine == NULL){
    //     // error("command line is null \n");
    //     return 0;
    // }

    // printf("%s \n", cmdLine);

    strncpy(startOfCommand, cmdLine, MAXLINE);
    startOfCommand += strspn(startOfCommand, special);




    endOfCommand = startOfCommand + strlen(startOfCommand);

    if(startOfCommand == endOfCommand){
        // write(STDERR_FILENO, error_message, strlen(error_message)); 
        parsedCmd->notRun =1;
        return 1;
    }

    realEndOfCommand = startOfCommand + strlen(startOfCommand);

    if(strchr(cmdLine,'>') != NULL && !strstr(cmdLine, "fi")){
        parsedCmd->ifRederect = 1;
        // printf("detected redirection\n");
        endOfCommand = strchr(startOfCommand,'>');
        if(endOfCommand == startOfCommand){
            write(STDERR_FILENO, error_message, strlen(error_message));
            parsedCmd->notRun = 1;
            return 0;
        }
        *endOfCommand = '\0';
        // printf("length %ld \n", endOfCommand - startOfCommand);
    }


    while(startOfCommand < endOfCommand){
        startOfCommand += strspn(startOfCommand, special);  //every empty command after special character from the beginning
        if(startOfCommand >= endOfCommand) break;
        skip = startOfCommand + strcspn(startOfCommand, special);
        *skip = '\0';    // make it end of the command string
        parsedCmd->cmd[numOfCommand] = startOfCommand;
        startOfCommand = skip + 1;
        numOfCommand++;
        if(numOfCommand > MAXARG) break;
    }
    

    // printf("num of command %d \n", numOfCommand);
    // printf("finished paring\n");



    // // printf("this is second command '%s' \n", parsedCmd->cmd[1]);
    // if(parsedCmd->ifRederect == 1){
    //     parsedCmd->cmd[fileNameStartPosition] = NULL;
    // }

    parsedCmd->cmd[numOfCommand] = NULL;
    parsedCmd->numberOfCommand = numOfCommand;

    // for(int i = 0; i != numOfCommand; i++){
        // printf(" %d th command : %s \n",i, parsedCmd->cmd[i]);
    // }


    if(parsedCmd->ifRederect == 1){
        startOfCommand = endOfCommand;
        endOfCommand = realEndOfCommand;
        numOfCommand = 0;
        

         while(startOfCommand < endOfCommand){
            startOfCommand += strspn(startOfCommand, special);  //every empty command after special character from the beginning
            if(startOfCommand >= endOfCommand) break;
            skip = startOfCommand + strcspn(startOfCommand, special);
            *skip = '\0';    // make it end of the command string
            parsedCmd->redirectFile = startOfCommand;
            startOfCommand = skip + 1;
            numOfCommand++;
            if(numOfCommand > MAXARG) break;
        }

        // printf("Rederecting file %s numsber is %d \n ", parsedCmd->redirectFile, numOfCommand);
        // printf("%ld\n", strstr(cmdLine, "fi") - cmdLine);
        if((numOfCommand == 1 ||numOfCommand > 2) && !strstr(cmdLine, "fi")){
            // printf("error in redrecting\n");
            write(STDERR_FILENO, error_message, strlen(error_message)); 
            // printf("end \n");
            parsedCmd->notRun = 1;
        }
    }
    //  printf(" really finished paring\n");
    return 1;
}


int exit2(){
    // printf("this is exit command \n");
    exit(0);
    // return 0;
}

int cd(char* dir){
    // printf("this is cd command \n");
    int result = chdir(dir);
    if(result == -1){
        // printf("No such folder: %s \n", dir);
        write(STDERR_FILENO, error_message, strlen(error_message)); 
    }
    return result;
}

int path(char** p){
    // env = {"/usr/bin/"};
    for(int i = 1; i < numOfCommand; i++){
        // env[i-1]= p[i];
        // printf("%s %s  \n", env[i-1], p[i]);
        strcpy(env[i-1], p[i]);
        // printf("success add %s \n", p[i]);
    }
    envNum = numOfCommand-1;
    return 1;
}







int if2(struct cmdWithArg* parsedCmd){
    int locationOfOperator = -1;
    int locationOfThen = -1;
    int locationOfFi = -1;

    int numberOfIf = 0;
    int numberOfThen = 0;
    int numberOfFi = 0;

    int operator;
    struct cmdWithArg* firstCmd = malloc(sizeof(struct cmdWithArg));
    struct cmdWithArg* secondCmd = malloc(sizeof(struct cmdWithArg));
    firstCmd->ifRederect = 0;
    secondCmd->ifRederect =0;

    for(int i = 0; i < parsedCmd->numberOfCommand; i++){
        // printf("second : %s \n", parsedCmd->cmd[i] );
        if(locationOfOperator <0 && (strcmp(parsedCmd->cmd[i], "==") == 0 || strcmp(parsedCmd->cmd[i], "!=") == 0)){
            // printf("find operator\n");
            locationOfOperator = i;
        }
        if(locationOfThen < 0 && (strcmp(parsedCmd->cmd[i], "then")) == 0){
            locationOfThen = i;
        }

        if((strcmp(parsedCmd->cmd[i], "then")) == 0){
            numberOfThen++;
        }

        if((strcmp(parsedCmd->cmd[i], "if")) == 0){
            numberOfIf++;
        }

        if((strcmp(parsedCmd->cmd[i], "fi")) == 0){
            numberOfFi++;
            locationOfFi = i;
        }

    }

    // printf("location of then %d \n", locationOfThen);
    // printf("location of fi %d\n", locationOfFi);
    if(locationOfThen - locationOfFi == -1){
        parsedCmd->notRun = 1;
        return 1;
    }
    // locationOfFi = parsedCmd->numberOfCommand -1;
    // printf("%s \n"; parsedCmd->cmd[parsedCmd->numberOfCommand - 1]);

    if(parsedCmd->numberOfCommand - locationOfFi != 1){
        // printf("last one is not fi");
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        parsedCmd->notRun = 1;
        return 1;
    }

    if(locationOfOperator == -1){
        // printf("sdfsf");
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        return -1;
    }

    if(numberOfFi != numberOfIf || numberOfFi != numberOfThen || numberOfIf != numberOfThen){
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        return -1;
    }

    for(int i = 1; i < locationOfOperator; i++){
        char* individualCommand = malloc(MAXARG);
        strcpy(individualCommand, parsedCmd->cmd[i]);
        firstCmd->cmd[i-1] = individualCommand;
        // printf("first command %s \n", individualCommand);
    }
    firstCmd->cmd[locationOfOperator-1]= NULL;

    int redirected = 0;
    for(int i = locationOfThen + 1; i < locationOfFi ; i++){
        char* individualCommand = malloc(MAXARG);
        strcpy(individualCommand, parsedCmd->cmd[i]);

        if(strcmp(individualCommand, ">") == 0){
            // printf("redirected\n");
            secondCmd->cmd[i - locationOfThen - 1] = NULL;
            redirected = 1;
            secondCmd->ifRederect = 1;
        }
        if(redirected == 0){
            secondCmd->cmd[i - locationOfThen - 1] = individualCommand;
            // printf("second command %s \n", individualCommand);
            secondCmd->numberOfCommand++;
        }
        else{
            secondCmd->redirectFile = individualCommand;
        }

    }
    secondCmd->cmd[locationOfFi - locationOfThen - 1] = NULL;

    int returnValue, compareValue;
    // printf("start running first command  %s\n", firstCmd->cmd[0]);
    returnValue = runCommand(firstCmd);
    // printf("finish running first command\n");
    compareValue = atoi(parsedCmd->cmd[locationOfOperator+1]);
    if(strcmp(parsedCmd->cmd[locationOfOperator], "==") == 0 && returnValue == compareValue){
        // printf("run second command\n");
        return runCommand(secondCmd);
    }
    else if(strcmp(parsedCmd->cmd[locationOfOperator], "!=") == 0 && returnValue != compareValue){
        // printf("run second command\n");
        return runCommand(secondCmd);
    }
    return 0;
}

// int if2(char** cmd){
//     if(strcmp(cmd[0], "if") != 0){
//         printf("there is an error at if command");
//         exit2();
//     }
//     char firstCommand[MAXLINE];
//     char** secondCommand;
//     int operator;    // 0 is equal, 1 is non-equal;
//     int compareValue;
    
//     strcpy(firstCommand, cmd[1]);
//     if(strcmp(cmd[2], "==") == 0){
//         operator = 0;
//     }
//     else if(strcmp(cmd[2], "!=") == 0){
//         operator = 1;
//     }
//     else{
//         printf("there is an error at if command");
//         exit2();
//     }

//     compareValue = atoi(cmd[3]);

//     // struct cmdWithArg *firstCmdWithArg = malloc(sizeof(struct cmdWithArg));
//     // strcpy(firstCmdWithArg->cmd[0], firstCommand);
//     // if(operator == 0){
        
//     // }


    

// }
