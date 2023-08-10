#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
void handler(int sig){
    sig = sig;
}
void test(char **args){
    int i = 0;
    while (args[i] != NULL){
        if (strcmp(args[i], "<<") == 0){
            fprintf(stderr, "Error: invalid command\n");
            exit(0);
        }
        else if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0){
            if (i == 0 || args[i + 1] == NULL){
                fprintf(stderr, "Error: invalid command\n");
                exit(0);
            }
            int fd;
            if (strcmp(args[i], ">") == 0){
                fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
            }
            else{
                fd = open(args[i + 1], O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
            }
            args[i] = NULL;
            dup2(fd, 1);
            close(fd);
        }
        else if (strcmp(args[i], "<") == 0){
            if (i == 0 || args[i + 1] == NULL){
                fprintf(stderr, "Error: invalid command\n");
                exit(0);
            }
            int fd = open(args[i + 1], O_RDONLY, S_IRUSR | S_IWUSR);
            if (fd == -1){
                fprintf(stderr, "Error: invalid file\n");
                exit(0);
            }
            args[i] = NULL;
            dup2(fd, 0);
            close(fd);
        }
        i++;
    }
    if (execvp(args[0], args) == -1){
        fprintf(stderr, "Error: invalid program\n");
        exit(0);
    }
}
int main(){
    setenv("PATH", "/usr/bin", 1);
    char cwd_bf[1000];
    size_t bufsize = 1000;
    char *gl_buffer = malloc(bufsize);
    char *originalBuffer = malloc(bufsize);
    pid_t job_ids[101];
    char job_command[101][1000];
    int jobStart = 1;
    fflush(stdout);
    for (int i = 0; i < 101; i++){
        job_ids[i] = -1;
        strcpy(job_command[i], "");
    }
    while (1){
        signal(SIGINT, handler);
        signal(SIGTSTP, handler);
        getcwd(cwd_bf, 1000);
        char *slashPtr = strrchr(cwd_bf, '/');
        if (*(slashPtr + 1) == '\0'){
            printf("[nyush %s]$ ", slashPtr);
        }
        else{
            printf("[nyush %s]$ ", slashPtr + 1);
        }
        fflush(stdout);
        ssize_t gl_res = getline(&gl_buffer, &bufsize, stdin);
        strcpy(originalBuffer, gl_buffer);
        if (gl_res == -1){
            printf("\n");
            exit(0);
        }
        int argLen = 0;
        char *strtok_res = strtok(gl_buffer, " ");
        char *args[bufsize];
        while (strtok_res != NULL){
            if ((strcmp(strtok_res, "\n") == 0) || (strcmp(strtok_res, " ") == 0)){
                strtok_res = strtok(NULL, " ");
                continue;
            }
            args[argLen] = strtok_res;
            int j = 0;
            while (args[argLen][j] && args[argLen][j] != '\n'){
                j++;
            }
            args[argLen][j] = '\0';
            strtok_res = strtok(NULL, " ");
            argLen++;
        }
        if (argLen == 0){
            continue;
        }
        args[argLen] = NULL;
        int status;
        int parser = 0;
        for (int i = 0; i < argLen; i++){
            if (strcmp(args[i], "|") == 0)
            {
                parser = 1;
                if (i == 0 || i == argLen - 1)
                {
                    fprintf(stderr, "Error: invalid command\n");
                    parser = -1;
                    break;
                }
            }
        }
        if (parser == -1){
            continue;
        }
        if (strcmp(args[0], "exit") == 0){
            if (argLen != 1){
                fprintf(stderr, "Error: invalid command\n");
                continue;
            }
            else{
                int flag = 1;
                for (int i = 0; i < 101; i++){
                    if (job_ids[i] != -1){
                        fprintf(stderr, "Error: there are suspended jobs\n");
                        flag = -1;
                        break;
                    }
                }
                if (flag == 1){
                    exit(0);
                }
            }
        }
        else if (strcmp(args[0], "fg") == 0){
            if (argLen != 2){
                fprintf(stderr, "Error: invalid command\n");
                continue;
            }
            int index = atoi(args[1]);
            int pid = job_ids[index];
            char oldCommand[100];
            strcpy(oldCommand, job_command[index]);
            if (index > 100 || job_ids[index] == -1 || kill(job_ids[index], SIGCONT) == -1){
                fprintf(stderr, "Error: invalid job\n");
                continue;
            }
            for (int j = index; j < 101; j++){
                if (j == 100 || job_ids[j + 1] == -1){
                    job_ids[j] = -1;
                    strcpy(job_command[j], "");
                    break;
                }
                job_ids[j] = job_ids[j + 1];
                strcpy(job_command[j], job_command[j + 1]);
            }
            jobStart--;
            pid_t result = waitpid(pid, &status, WUNTRACED);
            if (WIFSTOPPED(status)){
                if (jobStart > 100){
                    fprintf(stderr, "Error: jobs list full\n");
                    continue;
                }
                job_ids[jobStart] = result;
                strcpy(job_command[jobStart], oldCommand);
                jobStart++;
            }
        }
        else if (strcmp(args[0], "jobs") == 0)
        {
            for (int i = 1; i < 101; i++){
                if (job_ids[i] == -1){
                    break;
                }
                printf("[%d] %s", i, job_command[i]);
            }
        }
        else if (strcmp(args[0], "cd") == 0){
            if (argLen != 2){
                fprintf(stderr, "Error: invalid command\n");
            }
            else if (chdir(args[1]) == -1){
                fprintf(stderr, "Error: invalid directory\n");
            }
        }
        else{
            if (parser == 1){
                char *cmd[100][100];
                int num_arg = 0;
                int cmd_index = 0;
                for (int i = 0; args[i] != NULL; i++){
                    if (strcmp(args[i], "|") == 0){
                        cmd[cmd_index][num_arg] = NULL;
                        num_arg = 0;
                        cmd_index++;
                        continue;
                    }
                    cmd[cmd_index][num_arg] = args[i];
                    num_arg++;
                }
                cmd[cmd_index][num_arg] = NULL;
                int i = 0;
                int fd[2];
                int temp_pipe;
                int pid1;
                while (i < cmd_index){
                    pipe(fd);
                    pid1 = fork();
                    if (pid1 == 0){
                        if (i != 0){
                            dup2(temp_pipe, STDIN_FILENO);
                        }
                        dup2(fd[1], STDOUT_FILENO);
                        test(cmd[i]);
                        exit(0);
                    }
                    close(fd[1]);
                    temp_pipe = fd[0];
                    i++;
                }
                int pid = fork();
                if (pid == 0){
                    dup2(temp_pipe, STDIN_FILENO);
                    test(cmd[i]);
                    exit(0);
                }
                waitpid(pid, &status, WUNTRACED);
                waitpid(pid1, &status, WUNTRACED);
                continue;
            }
            pid_t pid = fork();
            if (pid == 0){
                test(args);
            }else{
                pid_t result = waitpid(pid, &status, WUNTRACED);
                if (WIFSTOPPED(status)){
                    if (jobStart > 100)
                    {
                        fprintf(stderr, "Error: jobs list full\n");
                        continue;
                    }
                    job_ids[jobStart] = result;
                    strcpy(job_command[jobStart], originalBuffer);
                    jobStart++;
                }
            }
        }
    }
}
