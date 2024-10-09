# Pseudocode for Linux Shell

## #1 Core Shell Functionality

1. Set up shell
   - Make constants and variables
   - List functions we'll use

2. Main loop
   - While shell is on:
     - Show prompt
     - Get user input
     - If user wants to quit, stop loop
     - Clean up input (remove extra spaces, split into parts)
     - If there are parts:
       - Check if it's a built-in command
         - If yes, do the built-in command
         - If it's "exit", stop the loop
       - If not, try to run it as a program

3. Do built-in commands
   - For each built-in command:
     - Exit: Close the shell
     - Help: Show list of commands
     - PWD: Show current folder
     - CD: Change folder
   - Handle any problems

## #2 Run Other Programs

4. Run a program
   - Make a new process
   - In the new process:
     - Find the program
     - If found, run it
     - If not found or can't run, show error
   - In the main process:
     - Wait for the new process to finish

5. Find program location
   - Check if program name has a path
     - If yes, make sure we can run it
   - If no path, look in system folders:
     - Check each folder in PATH
     - See if we can run the program there
   - If found, return full path. If not, say we couldn't find it

## #3 Handle Pipes and Redirection

6. Split input into commands
   - Look for pipe symbols (|)
   - Split input at pipe symbols
   - For each part:
     - Split into words

7. Set up pipes
   - For each command except the last:
     - Make a pipe

8. Run piped commands
   - For each command:
     - Set input (from previous pipe or normal input)
     - Set output (to next pipe or normal output)
     - Check for input/output redirection (< or >)
     - Run the command (built-in or program)

## #4 Background Jobs

9. Check for background job
   - Look for & at end of command
   - If found, mark as background job

10. Handle background jobs
    - If it's a background job:
      - Don't wait for it to finish
      - Add to list of background jobs
    - If not:
      - Wait for job to finish

11. Wait command
    - When user types "wait":
      - Wait for all background jobs to finish

## #5 Error Handling

12. Handle errors
    - For each action:
      - Check if it worked
      - If not, show error message
    - Keep shell running even if a command fails