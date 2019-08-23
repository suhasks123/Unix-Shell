#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <sys/wait.h>
#include<fcntl.h>

char *inputfile;
char *outputfile;


void shell_loop();
char* shell_readline();
void outputredirect(char*);
void inputredirect(char*);
char **shell_splitcommand(char*);
char **shell_splitpipes(char*);
void shell_execute(char **args);

void builtin(char* cmd);

int ctr=0;
int rec;
int pipepresence = 0;
pid_t firstchild;

int main()
{
    shell_loop();

    return 0;
}


void shell_loop()
{
    char *line;
    int status;

    do
    {
        ctr = 0;
        outputfile = NULL;
        inputfile = NULL;
        char pth[100], host[100], user[100];
        gethostname(host, 100);
        getlogin_r(user, 100);
        printf("%s@%s", user, host);
        printf(":%s>> ", getcwd(pth, 100));
        line = shell_readline();
        int k=0;
        int ird = -1;
        int ord = -1;
        while(line[k]!='\0')
        {
            if(line[k]=='<')
                ird = k;
            else if(line[k]=='>')
                ord = k;
            k++;
        }
        if(ird<ord)
        {
            outputredirect(line);
            if(ird!=-1)
            {
                inputredirect(line);
            }
        }
        else if(ird>ord)
        {
            inputredirect(line);
            if(ord!=-1)
            {
                outputredirect(line);
            }
        }



        char **cmds;
        cmds = shell_splitpipes(line);
        if(ctr==1)
        {
            char temp[100];
            strcpy(temp, *cmds);
            char **args;
            args = shell_splitcommand(temp);
            if(strcmp(*args,"cd")==0)
            {
                if(chdir(*(args+1))<0)
                {
                    perror(*(args+1));
                }
                free(line);
                free(cmds);
                continue;
            }
            if(strcmp(*args,"pwd")==0)
            {
                printf("%s\n", pth);
                free(line);
                free(cmds);
                continue;
            }
            if(strcmp(*args,"exit")==0)
            {
                free(line);
                free(cmds);
                exit(0);
            }
        }

        rec = ctr;
        shell_execute(cmds);
        //status = shell_execute(args);



        free(line);
        free(cmds);
    }while(1);
}

void shell_execute(char **cmds)
{
    pid_t child_pid;
    pid_t w_pid;
    int status;
    int outputfd, inputfd;
    int pfd[2];
    pipe(pfd);

    child_pid = fork();
    if(child_pid==0)
    {
        if(rec!=0)
        {
            rec--;
            shell_execute(cmds);
        }
        if(cmds[rec+1]!=NULL)
        {
            close(pfd[0]);
            dup2(pfd[1], 1);
        }
        if(rec==ctr-1)
        {
            if(outputfile!=NULL)
            {
                outputfd = open(outputfile, O_RDWR | O_CREAT | O_EXCL, 0666);
                dup2(outputfd, 1);
            }
        }
        if(rec==0)
        {
            if(inputfile!=NULL)
            {
                inputfd = open(inputfile, O_RDWR);
                dup2(inputfd, 0);
            }
            close(pfd[0]);
        }
        char **args;
        args = shell_splitcommand(cmds[rec]);
        int j;
        for(j=0;j<ctr;j++)
            printf("%s", args[j]);

        char pth[100];
        getcwd(pth, 100);

        if(strcmp(*args,"pwd")==0)
        {
            printf("%s\n", pth);
            close(pfd[1]);
            exit(0);
        }


        if(execvp(args[0], args)==-1)
        {
            close(pfd[1]);
            close(pfd[0]);
            perror("Shell");
        }
        close(pfd[1]);
        close(pfd[0]);
        if(close(outputfd)<0)
        {
            perror("file");
            exit(EXIT_FAILURE);
        }
        exit(0);

    }
    else if(child_pid<0)
        perror("Shell");
    else
    {
        if(rec!=ctr)
        {
            close(pfd[1]);
            dup2(pfd[0], 0);
        }

        //w_pid  = waitpid(child_pid, &status, WUNTRACED);

        do
        {
            w_pid  = waitpid(child_pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
        rec++;
    }
}

char *shell_readline()
{
    char *line = NULL;
    ssize_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

void outputredirect(char* line)
{
    char* token = strtok(line, ">");
    int i = 0;
    while(token != NULL)
    {
        if(i==1)
        {
            char* tokenws = strtok(token, " \n");
            outputfile = tokenws;
            return;
        }
        token = strtok(NULL, ">");
        i++;
    }
}

void inputredirect(char* line)
{
    char* token = strtok(line, "<");
    int i = 0;
    while(token != NULL)
    {
        if(i==1)
        {
            char* tokenws = strtok(token, " \n");
            inputfile = tokenws;
            return;
        }
        token = strtok(NULL, "<");
        i++;
    }
}

char **shell_splitpipes(char* line)
{
    int size = 64;
    char **args = malloc(size * sizeof(char*));
    char* token = strtok(line, "|\n");
    int i=0;
    ctr=0;
    while(token != NULL)
    {
        pipepresence = 1;
        args[i] = token;
        token = strtok(NULL, "|\n");
        i++;
        ctr++;
    }
    args[i]=NULL;
    return args;
}

char **shell_splitcommand(char* line)
{
    int size = 64;
    char **args = malloc(size * sizeof(char*));
    char* token = strtok(line, " \n\"");
    int i=0;
    while(token != NULL)
    {
        args[i] = token;
        token = strtok(NULL, " \n\"");
        i++;
    }
    args[i]=NULL;
    return args;
}
