#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include"main.h"
#include<signal.h>

void own_handler(int signum, siginfo_t *info, void *arg);

Slist *head = NULL;
char input_2[30],prompt_2[30];
int once = 1, flag = 0,flag_1 = 0,fd,fd_flag = 1;
int status,ret, i=0;
extern int child_pid;

void scan_input(char *input, char *prompt)
{
    struct sigaction new_act;
    memset(&new_act, 0, sizeof(new_act));
    new_act.sa_sigaction = own_handler;
    new_act.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &new_act, NULL);
    sigaction(SIGTSTP, &new_act, NULL);

    while(1)
    {
	once = 1;
	printf(ANSI_COLOR_GRAY"%s:~$"ANSI_COLOR_RESET, prompt);
	scanf("%[^\n]",input);
	strcpy(input_2,input);
	if(once == 1) 
	    getchar();
	if(!strncmp(input,"PS1=",4))
	{
	    if(input[4] !=' ')
		strcpy(prompt,&input[4]);
	}

	else if(strcmp(input,"")==0)
	    continue;

	if(strcmp(input,"echo $?")==0)
	    printf("%d\n",status);

	else if(strcmp(input,"echo $$")==0)
	    printf("%d\n",getpid());

	else if(strcmp(input,"echo $SHELL")==0)
	    system("pwd");


	char *cmd = get_command(input);
	int ret_val =  check_command_type(cmd);

	if(ret_val == EXTERNAL)
	    extract_external_commands(input);

	else if(ret_val == BUILTIN)
	    execute_internal_commands(input);

	else
	    printf("Command '%s' not found\n",cmd);
	memset(input, 0, 25);
    }
}


void own_handler(int signum, siginfo_t *info, void *arg)
{
    if(flag_1 == 0)
    {
	if(signum == SIGINT)
	    kill(child_pid,SIGINT);

	else if(signum == SIGTSTP)
	{
	    kill(child_pid,SIGTSTP);
	    if(flag == 1)
	    {
		printf("\n[%d]+ Stopped			%s",++i,input_2);
		insert_at_last(&head,info->si_pid,input_2);
	    }
	}
    }
    else
	printf("\n");
    flag_1 = 1;
    once = 0;
}


char *get_command(char *input_string)
{
    char *buffer = malloc(20*sizeof(char));
    strcpy(buffer,input_string);
    int i=0;
    while(buffer[i] != '\0')
    {
	if(buffer[i] ==' ')
	{
	    buffer[i] = '\0';
	    return buffer;
	}
	i++;
    }
    return buffer;
}


int check_command_type(char *command)
{
    char *builtins[] = {"echo", "printf", "read", "pushd", "popd", "dirs", "let", "eval","set","unset", "export", "declare", "typeset", "readonly", "getopts", "source","exec", "shopt","caller", "true", "type", "hash", "bind", "help", "exit", "cd", "fg", "bg", "jobs", NULL};

    int i = 0;

    char ch, buffer[100];

    while(builtins[i] != NULL)
    {
	if(strcmp(builtins[i], command) == 0)
	    return BUILTIN;
	i++;
    }
    if(fd_flag)
    {
	fd = open("external_command.txt",O_RDONLY);
	fd_flag = 0;
    }
    i=0;
    if(fd == -1)
	return -1;
    while (read(fd,&ch,1)>0)
    {
	if(ch!='\n')
	    buffer[i++]=ch;
	else 
	{
	    buffer[i] = '\0';
	    if(strcmp(buffer, command) == 0)
	    {
		lseek(fd,0,SEEK_SET);
		return EXTERNAL;
	    }
	    i=0;
	}
    }
    lseek(fd,0,SEEK_SET);
    return NO_COMMAND;
}
