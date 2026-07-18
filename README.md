# C-Shell

A mini Unix-like shell written in C for the OSN Mini Project 1 shell assignment.
The shell starts in the directory where `./shell.out` is run and shows prompts in
the format:

```text
<username@system:current_path>
```

`current_path` is shown relative to the shell home using `~` whenever possible.

## Features

- Runs normal system commands using `fork`, `execvp`, and `wait`.
- Parses command separators: `;` for sequential execution and `&` for background jobs.
- Supports I/O redirection with `<`, `>`, and `>>`.
- Supports command pipelines using `|`.
- Tracks background and stopped jobs.
- Handles `Ctrl-C`, `Ctrl-D`, and `Ctrl-Z` for foreground process control and logout.

## Built-in Commands

### `hop [path ...]`

Changes the current working directory. Multiple paths are handled from left to
right. With no argument or `~`, it moves to the shell home. `.` stays in the
same directory, `..` moves to the parent directory, and `-` switches to the
previous directory. Relative and absolute paths are supported. If the target
does not exist, it prints `No such directory!`.

### `reveal [flags] [path]`

Lists files and directories in lexicographic order. By default, hidden files are
not shown. Use `-a` to include hidden entries and `-l` to print one entry per
line. Flags can be combined, for example `reveal -la ~`. The path argument works
like `hop`, but only for listing contents. Invalid usage prints
`reveal: Invalid Syntax!`.

### `log`

Shows command history from oldest to newest. The shell stores up to 15 commands,
persists them across sessions, skips repeated consecutive commands, and does not
store `log` commands.

### `log purge`

Clears the saved command history.

### `log execute <index>`

Executes a command from history. Indexing is one-based from newest to oldest, so
`log execute 1` runs the most recent stored command. Invalid syntax prints
`log: Invalid Syntax!`.

### `activities`

Displays active jobs created by the shell. Each entry shows the process ID,
command name, and whether the job is `Running` or `Stopped`.

### `ping <pid> <signal_number>`

Sends a signal to the given process ID. The signal number is reduced modulo 32
before sending. If the process does not exist, it prints
`No such process found`.

### `fg [job_number]`

Brings a background or stopped job to the foreground. If no job number is given,
the most recent job is selected. A stopped job is resumed before being brought
forward. If the job is missing, it prints `No such job`.

### `bg [job_number]`

Resumes a stopped job in the background. If no job number is given, the most
recent job is selected. Already running or missing jobs are reported with an
error message.

## Build and Run

```bash
make clean
make all
./shell.out
```

## Project Structure

```text
include/     Header files
src/         Shell source files grouped by functionality
Makefile     Builds the shell.out executable
```
