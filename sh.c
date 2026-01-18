#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFF_SIZE 512
#define TOK_BUFF_SIZE 64
#define TOK_DELIM " \t\n"

void trim(char* line)
{
	line[strlen(line) - 1] = '\0';
}

char** get_args(char** tokens) 
{
	int count = 0; 
	
	while(tokens[count] != NULL && count < TOK_BUFF_SIZE)
	{
		count++;
	}
	
	char** args = malloc((count + 1)* sizeof(char*));
	if (!args) return NULL;
	
	for (int i = 0; i < count; i++)
	{
		args[i] = tokens[i];
	}
	
	args[count] = NULL;
	return args;
}

char** tokenize(char* line)
{
	int pos = 0;
	int buff_size = TOK_BUFF_SIZE;
	char** tokens = malloc(buff_size* sizeof(char*));
	char* token;
	
	if (!tokens) 
	{
		fprintf(stderr, "allocation error\n");
		exit(1);
	}
	
	token = strtok(line, TOK_DELIM);
	while(token != NULL)
	{
		tokens[pos] = token;
		pos++;
		
		if (pos >= buff_size)
		{
			buff_size += TOK_BUFF_SIZE;
			tokens = realloc(tokens, buff_size* sizeof(char*));
			if (!tokens) 
			{
				fprintf(stderr, "allocation error\n");
				exit(1);
			}
		}
		token = strtok(NULL, TOK_DELIM);
	}
	tokens[pos] = NULL;
	return tokens;
}

int sh_cd(char** args)
{
	if (args[1] == NULL)
	{
		fprintf(stderr, "Expected argument: cd <directory>\n");
	}
	else 
	{
		if (chdir(args[1]) != 0) perror("cd");
	}
	return 1;
}

void run(char** args)
{
	pid_t c_pid = fork();
	pid_t wpid;
	int status;
	if (c_pid == -1)
	{
		perror("Error");
		exit(1);
	}
	if (c_pid == 0)
	{
		if (execvp(args[0], args) == -1)
		{
			perror("Error");
		}
	}
	else 
	{
		while(!WIFEXITED(status))
		{
			wpid = waitpid(c_pid, &status, WUNTRACED);
		}
	}
}

int main(void)
{
	printf("Mini shell in C!\n");
	
	char* line;
	char** tokens;
	char** args;
	while(1) 
	{
		fflush(stdout);
		
		printf(">>> ");
		fgets(line, BUFF_SIZE, stdin);
		
		trim(line);
		tokens = tokenize(line);
		args = get_args(tokens);
		
		if (strcmp(args[0], "cd") == 0)
		{
			sh_cd(args);
			continue;
		}
		if (strcmp(line, "exit") == 0) break;
		
		run(args);
	}	
	return 0;
}