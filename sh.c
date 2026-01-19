#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFF_SIZE 512
#define TOK_BUFF_SIZE 64
#define TOK_DELIM " \t\n"

void trim(char* line)
{
	line[strlen(line) - 1] = '\0';
}

int get_args_length(char** args)
{
	int count = 0;
	while (args[count] != NULL && count < TOK_BUFF_SIZE) count++;
	return count;
}

int check_pipes(char** args)
{
	int length = get_args_length(args);
	for (int i = 0; i < length; i++)
	{
		if (strcmp(args[i], "|") == 0) return 1;
	}
	return 0;
}

char*** split_pipes(char** args) // input: {"ls", "-lh", "|", "cd", NULL}
{
	int num_cmds = 1;
	int len = get_args_length(args);
	for (int i = 0; i < len; i++)
	{
		if (strcmp(args[i], "|") == 0) 
			num_cmds++;
	}
	char*** cmds = malloc((num_cmds + 1) * sizeof(char**)); // should output: {{"ls", "-lh", NULL}, {"cd", NULL}};

	int cmd_index = 0;
	int start = 0;
	for (int i = 0; ;i++)
	{
		if (args[i] == NULL || strcmp(args[i], "|") == 0) 
		{
			int argc = i - start;
			cmds[cmd_index] = malloc((argc + 1) * sizeof(char*));
			for (int j = 0; j < argc; j++)
			{
				cmds[cmd_index][j] = args[start + j];
			}
			cmds[cmd_index][argc] = NULL;
            cmd_index++;
            if (args[i] == NULL) break;
            start = i + 1;
		}
	}
	cmds[cmd_index] = NULL;
    return cmds;
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

void run_single(char** args)
{
	pid_t c_pid = fork();
	int status;

	if (c_pid == -1)
	{
		perror("fork");
		exit(1);
	}

	if (c_pid == 0)
	{
		execvp(args[0], args);
		perror("Error");
		exit(1);
	}

	waitpid(c_pid, &status, 0);

}

void run_pipe(char*** commands)
{
	int pipefd[2];
	pid_t p1, p2;

	if (pipe(pipefd) < 0)
	{
		fprintf(stderr, "Pipe error");
		return;
	}

	p1 = fork();
	if (p1 < 0) 
	{
		perror("Error");
		exit(1);
	}
    if (p1 == 0)
    {
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) 
		{ 
			perror("dup2"); 
			exit(1); 
		}
        close(pipefd[1]);

        execvp(commands[0][0], commands[0]);
        perror("execvp");
        exit(1);
    }
	    p2 = fork();
    if (p2 < 0)
    {
        perror("fork");
        return;
    }

    if (p2 == 0)
    {
        close(pipefd[1]);
        if (dup2(pipefd[0], STDIN_FILENO) < 0) 
		{ 
			perror("dup2"); 
			exit(1); 
		}
        close(pipefd[0]);

        execvp(commands[1][0], commands[1]);
        perror("execvp");
        _exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
}

int main(void)
{
	printf("Mini shell in C!\n");
	
	char line[BUFF_SIZE];
	char** tokens;
	char** args;
	char*** commands;
	while(1) 
	{
		fflush(stdout);
		
		printf(">>> ");
		fgets(line, BUFF_SIZE, stdin);
		
		trim(line);
		tokens = tokenize(line);
		args = get_args(tokens);

		if (args[0] == NULL) continue;
		
		if (strcmp(args[0], "cd") == 0)
		{
			sh_cd(args);
			continue;
		}
		if (strcmp(args[0], "exit") == 0) break;
		
		if (check_pipes(args))
		{
			commands = split_pipes(args);
			run_pipe(commands);
		}
		else
		{
			run_single(args);
		}
	}	
	return 0;
}