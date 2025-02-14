/**********************************************************************
 * Copyright (c) 2020
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include<unistd.h>
#include <sys/wait.h>
#include "types.h"

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
/**/

#define MAX_NR_TOKENS	32	/* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN	128	/* Maximum length of single token */
#define MAX_COMMAND_LEN	4096 /* Maximum length of assembly string */

/*
 * String used as the prompt (see @main()). You may change its value to
 * change the prompt */
static char __prompt[MAX_TOKEN_LEN] = "$";

/**
 *          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/


/***********************************************************************
 * parse_command()
 *
 * DESCRIPTION
 *  Parse @command, and put each command token into @tokens[] and the number of
 *  tokes into @nr_tokens. You may use this implemention or your own from PA0.
 *
 *  A command token is defined as a string without any whitespace (i.e., *space*
 *  and *tab* in this programming assignment). For exmaple,
 *   command = "  cp  -pr /home/sslab   /path/to/dest  "
 *
 *  then, nr_tokens = 4, and tokens is
 *    tokens[0] = "cp"
 *    tokens[1] = "-pr"
 *    tokens[2] = "/home/sslab"
 *    tokens[3] = "/path/to/dest"
 *    tokens[>=4] = NULL
 *
 * RETURN VALUE
 *  Return 1 if @nr_tokens > 0
 *  Return 0 otherwise
 */
int parse_command(char *command, int *nr_tokens, char *tokens[])
{
	char *curr = command;
	int isQuote=false;
	int token_started = false;
	*nr_tokens = 0;
	while (*curr != '\0') 
	{  
		if (isspace(*curr)&&!(isQuote)) {  
			*curr = '\0';
			token_started = false;
		}
		else if (*curr=='"')
		{
			if (isQuote==false)
            isQuote=true;
            else{
            isQuote=false;
			*curr='\0';
			}
		}
		 else {
			if (!token_started) {
				tokens[*nr_tokens] = curr;
				*nr_tokens += 1;
				token_started = true;
			}
		}

		curr++;
	}

	return (*nr_tokens > 0);
}



static int run_command(int nr_tokens, char *tokens[])
{
	/* This function is all yours. Good luck! */
	pid_t pid;

	if (strncmp(tokens[0], "exit", strlen("exit")) == 0) 
	{
		return 0;
	}

	
	else if(strcmp(tokens[0],"prompt")==0)
	{
		strcpy(__prompt,tokens[1]);
		
	}

	else if(strcmp(tokens[0],"cd")==0)
	{

		if(strncmp(tokens[1],"~",1)==0)
		{
			chdir(getenv("HOME"));
		}
		else
		{
			chdir(tokens[1]);
		}

	}

	else if(strcmp(tokens[0],"for")==0)
	{
		int cnt=1;
		int target=1;
		int isFirst=true;

		for(int i=0;i<nr_tokens;i++)
		{
			if(strcmp(tokens[i],"for")==0)
			{
				cnt*=atoi(tokens[i+1]);
			}

			else if(!(strcmp(tokens[i],"for")==0)&&(!(isdigit(*tokens[i])))&&(isFirst))
			{
				target = i;
				isFirst=false;
			}
			else
			{
				continue;
			}
			
		}

		if(strcmp(tokens[target],"cd")==0)
		{
			for(int i=0;i<cnt;i++)
			chdir(tokens[target+1]);
		}

		else
		{
				for(int i=0;i<cnt;i++)
				{
					pid=fork();
					if(pid==0)
					{
						execvp(tokens[target],&tokens[target]);
						exit(EXIT_SUCCESS);
					}
				
					else if(pid==-1)
					{
						fprintf(stderr,"Error occured\n");
						exit(EXIT_FAILURE);
					}
					else
					{
						wait(NULL);
					}
				}
		}
		

	}
	
	else
	{
		pid=fork();
		if(pid==0){
			execvp(tokens[0],tokens);
			if(strlen(tokens[0])!=0)
			{
				fprintf(stderr, "No such file or directory\n");
				close(pid);
			}
			
			exit(EXIT_SUCCESS);

		}
		else if(pid==-1)
		{
			fprintf(stderr,"Error occured\n");
			exit(EXIT_FAILURE);
		}

		else
		{
			wait(NULL);
		}
		
	}

	return 1;
}


/***********************************************************************
 * initialize()
 *
 * DESCRIPTION
 *   Call-back function for your own initialization code. It is OK to
 *   leave blank if you don't need any initialization.
 *
 * RETURN VALUE
 *   Return 0 on successful initialization.
 *   Return other value on error, which leads the program to exit.
 */
static int initialize(int argc, char * const argv[])
{
	return 0;
}


/***********************************************************************
 * finalize()
 *
 * DESCRIPTION
 *   Callback function for finalizing your code. Like @initialize(),
 *   you may leave this function blank.
 */
static void finalize(int argc, char * const argv[])
{

}


/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING BELOW THIS LINE ******      */

static bool __verbose = true;
static char *__color_start = "[0;31;40m";
static char *__color_end = "[0m";

/***********************************************************************
 * main() of this program.
 */
int main(int argc, char * const argv[])
{
	char command[MAX_COMMAND_LEN] = { '\0' };
	int ret = 0;
	int opt;

	while ((opt = getopt(argc, argv, "qm")) != -1) {
		switch (opt) {
		case 'q':
			__verbose = false;
			break;
		case 'm':
			__color_start = __color_end = "\0";
			break;
		}
	}

	if ((ret = initialize(argc, argv))) return EXIT_FAILURE;

	if (__verbose)
		fprintf(stderr, "%s%s%s ", __color_start, __prompt, __color_end);

	while (fgets(command, sizeof(command), stdin)) {	
		char *tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;

		if (parse_command(command, &nr_tokens, tokens) == 0)
			goto more; /* You may use nested if-than-else, however .. */

		ret = run_command(nr_tokens, tokens);
		if (ret == 0) {
			break;
		} else if (ret < 0) {
			fprintf(stderr, "Error in run_command: %d\n", ret);
		}

more:
		if (__verbose)
			fprintf(stderr, "%s%s%s ", __color_start, __prompt, __color_end);
	}

	finalize(argc, argv);

	return EXIT_SUCCESS;
}
