#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

// Function prototypes
void execute_builtin(char **args);  // TODO: Implement this function in the next commit

int main() {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];  // Array of pointers to store command arguments
    char *token;

    while (1) {
        printf("shell> ");
        fflush(stdout);

        // Read user input and stop when Ctrl+D is pressed
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Remove newline character from the input
        input[strcspn(input, "\n")] = 0;

        // Tokenize input into separate arguments
        int arg_count = 0;
        token = strtok(input, " ");  // Get the first token (word)
        while (token != NULL && arg_count < MAX_ARGS - 1) {
            args[arg_count++] = token;  // Store pointer to the token
            token = strtok(NULL, " ");  // Get the next token
        }
        args[arg_count] = NULL;  // Null-terminate the argument list

        if (arg_count > 0) {
            // Check if the command is a built-in function
            if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "help") == 0 ||
                strcmp(args[0], "pwd") == 0 || strcmp(args[0], "cd") == 0) {
                execute_builtin(args);
                if (strcmp(args[0], "exit") == 0) {
                    break;
                }
            } else {
                // TODO: Implement custom executable in next commit
                printf("Command not found: %s\n", args[0]);
            }
        }
    }

    return 0;
}

void execute_builtin(char **args) {
    printf("Built-in command execution not yet implemented.\n");
}