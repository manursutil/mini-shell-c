# Mini Shell C

A minimal Unix style shell written in C with command parsing and tokenization. It executes 
external programs via fork() + execvp() for child processes and supports command piping using `|`.

Built-in commands:
- cd <dir> -> change current working directory
- exit -> exit the shell
- Piping (|) to pass output from one command to another (Just one pipe)

## Example use

```bash
>>> ls -l
>>> pwd
>>> ls | grep ".c"
>>> cd
>>> exit
```

## Build and run

```bash 
gcc -Wall -Wextra -pedantic -std=c99 *.c -o sh 
./sh
```

## Motivation
Why this project?

This shell was built as a learning project to understand how Unix shells work internally, 
focusing on process control, system calls, and execution flow at the OS level.

## What's next?
1. Command chaining 