/*This is an implementation of the basic features of a Unix shell.
It was created as a recreuitment task for the Systems SIG of the Web Enthusiasts' Club, NITK.

Created By: Suhas K S (2nd Yr CSE).

*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<fcntl.h>

/*These character pointers store the filenames of the files to read from and write to
when there is need for input or output redirection in the entered command.
*/

char *inputfile;
char *outputfile;


void shell_loop();
char* shell_readline();
void outputredirect(char*);
void inputredirect(char*);
char **shell_splitcommand(char*);
char **shell_splitpipes(char*);
void shell_execute(char **args);

/*These variables store and track the number of pipes in the command. ctr contains
(no. of pipes + 1) and rec is used to control the recursive forking during execution.
*/

int ctr=0;
int rec;

/*The main function doesn't do much. It calls shell_loop() which executes everything else.
*/
int main()
{
    shell_loop();

    return 0;
}

/*shell_loop() - It is the function responsible for keeping the basic parent loop of the shell running.

*/
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

        /*Here, after reading the line, we check it for any < or > characters since they
        signify input and output redirection. If present, we call the functions inputredirect(line)
        and outputredirect(line) to extract the file names to read from or write to and store it in
        the global variables inputfile and outputfile
        */

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

        /*After removing I/O redirection instructions from the line, we check for pipes.
        If pipes (|) exist, we extract commands between the pipes and store them in a string array
        cmds. We use the function shell_splitpipes(line) to perform this.
        */

        char **cmds;
        cmds = shell_splitpipes(line);

        /* Here, we check if the given command is a builtin. It can be a builtin only when
        there are no pipes. Since ctr keeps track of the number of commands in a line and indirectly,
        the pipe count, we use it to check whether there are no pipes. And then we execute the builtins
        with system calls chdir() for cd, getcwd() for pwd and exit() for exit. The builtin help is used to
        display all the builtins.
        */


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
            if(strcmp(*args,"help")==0)
            {
                printf("An Implementation of a Unix-like Shell.\n");
                printf("These are the built-in commands (Type help to view them):\n");
                printf("cd <pathname>  : Changes current directory to the specified pathname.\n");
                printf("pwd  : Displays the path of the current working directory.\n");
                printf("exit  : Exits out of the current shell.\n\n");
                printf("Use man <command> for detailed help on individual commands.\n");
                free(line);
                free(cmds);
                continue;
            }
        }

        /*After checking for builtins, we finally execute the line by passing cmds into
        shell_execute()
        */


        rec = ctr;
        shell_execute(cmds);
        //status = shell_execute(args);

        /*After the execution is done, we free the dynamically allocated line and cmds
        */

        free(line);
        free(cmds);
    }while(1);
}


/*shell_execute() is the function that executes the given command. It can perform execution of
simple commands, I/O redirection and command piping.
*/

/*The function uses recursion to implement command piping. It starts execution in the
last command in the pipeline. It then calls itself till it reaches the first command.
A fork() system call is encountered during every function call. So each command
uses fork() to create a child process which does the same till the first command is reached.
Since that command has an input, it is executed and its output if piped to the input of its parent
process. This is done till the shell parent process is reached.
*/

void shell_execute(char **cmds)
{
    pid_t child_pid;
    pid_t w_pid;
    int status;
    int outputfd, inputfd;   //File descripters for the input and output files for I/O redirection
    int pfd[2];
    pipe(pfd);

    child_pid = fork();  //forking the parent process


    if(child_pid==0)      //This block is executed if the current process is a child of the running process
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


    else if(child_pid<0)            //Errors during forking
        perror("Shell");



    else                            //This block is executed if the running process is the parent after forking
    {
        if(rec!=ctr)
        {
            close(pfd[1]);
            dup2(pfd[0], 0);
        }

        do
        {
            w_pid  = waitpid(child_pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
        rec++;
    }
}

/*shell_readline() takes input in the shell
*/


char *shell_readline()
{
    char *line = NULL;
    ssize_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

/* The following two functions extract the filenames from the input line and stores them
for input and output redirection
*/

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

/*The following functions split input lines according to the pipes present and commands according
to the spaces present. Since each seperate command is divided according to spaces, quotes and newline,
we can observe that it doesn't support arguments with spaces
*/

char **shell_splitpipes(char* line)
{
    int size = 64;
    char **args = malloc(size * sizeof(char*));
    char* token = strtok(line, "|\n");
    int i=0;
    ctr=0;
    while(token != NULL)
    {
        args[i] = token;
        token = strtok(NULL, "|\n");
        i++;
        ctr++;
    }
    args[i]=NULL;
    return args;
}

/*This updated version of shell_splitcommand() introduces quoting, so that filenames 
with spaces can be used without any problems. In the previous version of the function, 
the command was split only with respect to spaces which eliminated any possibility of using 
filenames with spaces*/

char **shell_splitcommand(char* line)
{
    int size = 64;
    char temp[100];
    char **argsquo = malloc(size * sizeof(char*));
    char **args = malloc(size * sizeof(char*));
    char* token = strtok(line, "\"\n");
    int i=0;
    while(token != NULL)
    {
        argsquo[i] = token;
        token = strtok(NULL, "\"\n");
        i++;
    }
    argsquo[i] = NULL;
    int k = 0;
    int j;
    for(j=0;j<i;j++)
    {
        if(j%2!=0)
        {
            args[k] = argsquo[j];
            k++;
            continue;
        }
        token = strtok(argsquo[j], " ");
        while(token != NULL)
        {
            args[k] = token;
            token = strtok(NULL, " ");
            k++;
        }
    }
    args[k] = NULL;



    return args;
}

/*This is the previous version of the function*/

/*char **shell_splitcommand(char* line)
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
}*/
