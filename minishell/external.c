#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include"main.h"
#include<signal.h>


extern int ret,status,i,flag,flag_1;
extern char input_2[30];

extern Slist *head;

int del_pid,child_pid;

void extract_external_commands(char *external_commands)
{
    int count = 0;
    char *commands[100];

    char *token = strtok(external_commands, "|");
    while (token != NULL)
    {
	commands[count++] = token;
	token = strtok(NULL, "|");
    }

    int pipe_fd[2];
    int prev_pipe_fd[2];


    for (int i = 0; i < count; i++)
    {
	char *argv[50];
	int argc = 0;

	token = strtok(commands[i], " ");
	while (token != NULL)
	{
	    argv[argc++] = token;
	    token = strtok(NULL, " ");
	}
	argv[argc] = NULL;

	if (i < count-1)
	{
	    printf("Count is %d\n",count);
	    if (pipe(pipe_fd) == -1)
	    {
		perror("pipe");
		exit(EXIT_FAILURE);
	    }
	}

	pid_t pid = fork();
	if (pid > 0)
	{
	    flag_1 = 0;
	    if (i < count - 1)
		close(pipe_fd[1]);

	    if (i > 0)
		close(prev_pipe_fd[0]);

	    flag = 1;
	    while((ret = waitpid(pid, &status, WNOHANG | WUNTRACED)==0))
	    {}
	    if (WIFSTOPPED(status)){flag = 0;}
	    else if(WIFEXITED(status))
	    {
		flag = 0;
		ret =  WEXITSTATUS(status);
	    }
	    else
	    {
		ret = status;
		flag = 0;
	    }
	}
	else if (pid == 0)
	{
	    child_pid = getpid();
	    if (i > 0)
	    {
		dup2(prev_pipe_fd[0], 0);  
		close(prev_pipe_fd[0]);  
	    }

	    if (i < count - 1)
	    {
		dup2(pipe_fd[1], 1); 
		close(pipe_fd[1]);  
	    }

	    execvp(argv[0], argv);
	}

	if (i < count - 1)
	{
	    prev_pipe_fd[0] = pipe_fd[0];
	    prev_pipe_fd[1] = pipe_fd[1];
	}
    }
}



void execute_internal_commands(char *input_string)
{
    if(strcmp(input_string,"exit")==0)
	exit(0);
    else if(strncmp(input_string,"cd",2)==0)
	chdir(input_string+3);
    else if(strcmp(input_string,"pwd")==0)
	system("pwd");
    else if(strcmp(input_string,"fg")==0)
    {
	int result = sl_delete_list(&head,&del_pid);
	if(result == -1)
	    printf("-bash: fg: current: no such job\n");
	kill(del_pid,SIGCONT);
	waitpid(del_pid,NULL,WUNTRACED);
    }
    else if(strcmp(input_string,"bg")==0)
    {
	int result = sl_delete_list(&head,&del_pid);
	if(result == -1)
	    printf("-bash: bg: current: no such job\n");
	kill(del_pid,SIGCONT);
	waitpid(del_pid,NULL,WNOHANG);
    }
    else if(strcmp(input_string,"jobs")==0)
	print_list(head);
}

int insert_at_last(Slist **head, data_t pid, char *input_2)
{
    Slist *new = malloc(sizeof(Slist));
    if(!(new))
    {
	return FAILURE;
    }
    new->pid = pid;
    strcpy(new->command,input_2);
    new->link = NULL;
    if(*head==NULL)
    {
	*head=new;
	return SUCCESS;
    }
    else
    {
	Slist *temp=*head;
	while(temp->link!=NULL)
	{
	    temp=temp->link;
	}
	temp->link=new;
	return SUCCESS;
    }
}

int sl_delete_list(Slist **head,int *del_pid)
{
    i--;
    if (*head == NULL)
    {
	return FAILURE; 
    }
    if ((*head)->link == NULL)
    {
	*del_pid = (*head) -> pid;
	printf("%s\n", (*head)->command); 
	free(*head);
	*head = NULL; 
	return SUCCESS;
    }
    Slist *temp = *head;
    Slist *prev = NULL;

    while (temp->link != NULL)
    {
	prev = temp; 
	temp = temp->link; 
    }
    if (prev != NULL)
    {
	*del_pid = temp -> pid;
	printf("%s\n", temp->command); 
	prev->link = NULL; 
	free(temp); 
    }
    return SUCCESS;
}

void print_list(Slist *head)
{
    int i=0;
    if (head == NULL)
	return;
    else
    {
	while (head)
	{
	    printf("[%d]  Stopped		%s\n",++i,head->command);
	    head = head -> link;
	}
    }
}
