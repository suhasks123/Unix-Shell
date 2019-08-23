#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>

void shell_loop();
char* shell_readline();
char **shell_splitcommand(char*);
void shell_execute(char **args);

void builtin(char* cmd);

int ctr=0;

int main()
{
    shell_loop();

    return 0;
}


void shell_loop()
{
    char *line;
    char **args;
    int status;

    do
    {
        char pth[100], host[100], user[100];
        gethostname(host, 100);
        getlogin_r(user, 100);
        printf("%s@%s", user, host);
        printf(":%s>> ", getcwd(pth, 100));
        line = shell_readline();
        args = shell_splitcommand(line);

        if(strcmp(*args,"cd")==0)
        {
            if(chdir(*(args+1))<0)
            {
                perror(*(args+1));
            }
            continue;
        }

        if(strcmp(*args,"pwd")==0)
        {
            printf("%s\n", pth);
            continue;
        }
        if(strcmp(*args,"exit")==0)
        {
            return;
        }

        shell_execute(args);
        //status = shell_execute(args);

        free(line);
        free(args);
    }while(1);
}

void shell_execute(char **args)
{
    pid_t child_pid;
    pid_t w_pid;
    int status;

    child_pid = fork();
    if(child_pid==0)
    {
        if(execvp(args[0], args)==-1)
        {
            perror("Shell");
            exit(EXIT_FAILURE);
        }
    }
    else if(child_pid<0)
        perror("Shell");
    else
    {
        do
        {
            w_pid  = waitpid(child_pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

char *shell_readline()
{
    char *line = NULL;
    ssize_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}



char **shell_splitcommand(char* line)
{
    int size = 64;
    char **args = malloc(size * sizeof(char*));
    char* token = strtok(line, " \n\"");
    int i=0;
    ctr=0;
    while(token != NULL)
    {
        args[i] = token;
        token = strtok(NULL, " \n\"");
        i++;
        ctr++;
    }
    args[i]=NULL;
    return args;
}
