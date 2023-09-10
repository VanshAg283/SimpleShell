# SimpleShell
SimpleShell is a basic Unix-like shell implemented in C that allows users to execute commands and view command history. This shell provides a command-line interface for interacting with the operating system.

## Features
- Accepts and executes user commands.
- Supports basic commands, including file manipulation and process execution.
- Implements a command history feature.
- Supports command pipelines.

## Usage
Once SimpleShell is running, you can enter commands just like you would in a regular Unix shell. Here are some example commands:

- Execute a command
```
ls -l
```

- Pipe output of one command to another: `|`
```
cat fib.c | wc -l
```

- View command history
```
history
```
SimpleShell maintains a command history that you can access using the history command. But it can only handle a single pipe and cannot execute previous commands from history by referencing their history numbers.

- Terminating Simple Shell
```
exit
```
To exit SimpleShell, simply enter the exit command, or use Ctrl-C to terminate the shell.