# C-Shell
Shell made using C language.
## Functionalities
On starting, the shell to display the below prompt
```<Username@SystemName:current_path>```
### Commands:
- all the default bash commands
- <bold>hop</bold>
    - hop ((~ | . | .. | - | name)*)?
    - ”~” or No Arguments: Change the CWD to the shell’s home directory.
    - ”.”: Do nothing (i.e. stay in the same CWD)
    - ”..”: Change the CWD to the parent directory of the CWD, or do nothing if the CWD has no parent directory.
    - ”-“: Change the CWD to the previous CWD or do nothing if there was no previous CWD. So, after starting the shell, till the first hop command which was not a “-“ was run, you must do nothing.
    - “name”: Change the CWD to the specified relative or absolute path.
    - If the directory does not exist, outputs “No such directory!”
## How to Run
- ```make clean```<br>
- ```make all```<br>
- ```./shell.out```<br>
