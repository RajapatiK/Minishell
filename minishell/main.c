#include<stdio.h>
#include<stdlib.h>
#include"main.h"

void scan_input(char *input, char *prompt);


int main()
{
    system("clear");

    char input_string[25];

    char prompt[25] ="minishell";

    scan_input(input_string, prompt);
}

