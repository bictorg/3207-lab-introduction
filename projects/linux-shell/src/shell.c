#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

// Function prototypes
void execute_builtin(char **args);

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
                printf("Command not found: %s\n", args[0]);
            }
        }
    }

    return 0;
}

void execute_builtin(char **args) {
    // Execute built-in commands
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
    } else if (strcmp(args[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  exit - Exit the shell\n");
        printf("  help - Display this help message\n");
        printf("  pwd - Print current working directory\n");
        printf("  cd <directory> - Change current working directory\n");
    } else if (strcmp(args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else if (chdir(args[1]) != 0) {
            perror("chdir");
        }
    }
}