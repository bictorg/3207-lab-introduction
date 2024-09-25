# Pseudocode for Linux Shell

## Deliverable #1: Core Shell Functionality

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