# Pseudocode for Linux Shell

## #1 Core Shell Functionality

1. Initialize shell
   - Set up necessary constants and variables
   - Declare function prototypes

2. Main loop
   - While shell is running:
     - Display shell prompt
     - Read user input
     - If input indicates end of input (e.g., EOF), exit loop
     - Process input (remove newline, tokenize into arguments)
     - If there are arguments:
       - Check if the command is a built-in
         - If yes, execute built-in command
         - If command is exit command, break the main loop
       - Else, attempt to execute as an external program

3. Built-in command execution
   - Implement logic for each built-in command:
     - Exit: Terminate the shell
     - Help: Display available commands and their usage
     - Print Working Directory: Show current directory
     - Change Directory: Change to specified directory
   - Handle errors and edge cases for each command

## #2 External Program Execution

4. Execute external program
   - Create a new process
   - In child process:
     - Locate the program in the system PATH
     - If found, execute the program
     - Handle errors (program not found, execution failed)
   - In parent process:
     - Wait for child process to complete

5. Program path resolution
   - Check if the program name includes a path
     - If yes, verify if it's executable
   - If no path provided, search in system PATH:
     - Iterate through PATH directories
     - Check for executable in each directory
   - Return full path if found, or indicate failure