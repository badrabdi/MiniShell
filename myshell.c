#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define MAX_LINE 80 /* The maximum length command */
char *hardCode[] = {"exit", "cd","!!"};

int prev = 0;
char *history[MAX_LINE/2+1];
int hSize = 0;

int main(void) {

    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    int should_run = 1; /* flag to determine when to exit program */
    char* path= "PATH=/bin/";
    int status;

    int count = 0;


    while (should_run) {

        printf("mysh:~$ ");

        fflush(stdout);
        int input = 0;
        int output = 0;
        int toPipe = 0;

        //Implementation of reading line inspired by linux docs: https://man7.org/linux/man-pages/man3/getline.3.html
        char *line = NULL;
        size_t len = 0;
        ssize_t nread;
        nread = getline(&line, &len, stdin);

        //Implementaion of splitting with space inspired from: https://www.educative.io/edpresso/splitting-a-string-using-strtok-in-c

        char *tok1 = strtok(line,"\n");
        char *token = strtok(tok1, " ");
        int index = 0;

        while (token != NULL) {
            args[index] = token;
            token = strtok(NULL, " ");
            index++;
        }

        args[index] = NULL;
        if(args[0] == NULL){
            continue;
        }
        int i = 1;
        prev = 0;
        if(strcmp(args[0],"!!") == 0){
            i = 1;
            prev = 1;
            int s = sizeof(history) /sizeof (char *);
            for(int j = 0; j <= hSize; j++){
                args[j] = history[j];
            }
            index = hSize;
        }
        for(int x = 0; x < index; x++){

            if(strcmp(args[x], ">") == 0){
                output = x +1;
            }
            if(strcmp(args[x],"<") == 0){
                input = x+1;
            }
            if(strcmp(args[x],"|") == 0){

                toPipe = x;
            }

        }
        for(int x = 0; x < 3; x++){
            if(strcmp(args[0],hardCode[x]) == 0){
                i = 0;
                if(strcmp(hardCode[x],"exit") == 0){
                    return 0;
                }
                else{
                    if(chdir(args[1]) != 0 ){
                        perror("here error");
                    }
                }
            }
        }


        if(i){

            int t = strcmp(args[0], "!!");
            pid_t pid,wpid;
            pid = fork();
            int s = 0;



            if(pid < 0){
                fprintf(stderr,"Couldn't Fork");
            }
            else if(pid == 0) {

                if (toPipe) {
                    char *args_first[MAX_LINE / 2 + 1];
                    char *args_last[MAX_LINE / 2 + 1];


                    for (int j = 0; j < toPipe; j++) {
                        args_first[j] = args[j];

                    }
                    args_first[toPipe] = NULL;
                    int temp = toPipe + 1;

                    for (int j = 0; j <= index - toPipe; j++) {
                        args_last[j] = args[temp];
                        temp++;

                    }



                    int fd[2];
                    pid_t childPid;
                    if (pipe(fd) != 0)
                        perror("failed to create pipe");

                    if ((childPid = fork()) == -1)
                        perror("failed to fork");

                    if (childPid == 0)
                    {
                        dup2(fd[1], 1);
                        close(fd[0]);
                        close(fd[1]);
                        execvp(args_first[0], args_first);
                        perror("failed to exec command 1");
                    }
                    else
                    {
                        dup2(fd[0], 0);
                        close(fd[0]);
                        close(fd[1]);
                        execvp(args_last[0], args_last);
                        perror("failed to exec command 2");
                    }

                }

                 else {
                    if (input) {

                        FILE *file_pointer;

                        file_pointer = fopen(args[input], "r");

                        if (file_pointer == NULL) {
                            perror(args[input]);
                            exit(EXIT_FAILURE);
                        }

                        int file_descriptor = fileno(file_pointer);
                        dup2(file_descriptor, 0);
                        fclose(file_pointer);
                        args[input - 1] = NULL;
                        args[input] = NULL;


                    }
                    if (output) {

                        FILE *file_pointer;

                        file_pointer = fopen(args[output], "ab+");

                        if (file_pointer == NULL) {
                            perror(args[input]);
                            exit(EXIT_FAILURE);
                        }

                        int file_descriptor = fileno(file_pointer);

                        dup2(file_descriptor, 1);

                        fclose(file_pointer);
                        args[output - 1] = NULL;
                        args[output] = NULL;


                    }

                    if (execvp(args[0], args) == -1) {
                        perror(args[0]);

                    }

                    exit(EXIT_FAILURE);
                }
            }
            else{

                wait(NULL);
            }



        }

        if(prev == 0){
            char *tok2 = strtok(line,"\n");
            char *token2 = strtok(tok2, " ");
            int i2 = 0;

            for(int ip = 0; ip < index; ip++){

                history[ip] = args[ip];

            }
            history[index] = NULL;



            hSize = index;
        }




        /** After reading user input, the steps are:
         * (1) fork a child process using fork()
         * (2) the child process will invoke execvp()
         * (3) parent will invoke wait() unless command included &
         */
        fflush(stdin);

        count++;
    }



    return 0;
}


