#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "helpers.h"

#define MAX_INPUT_SIZE 1024

// Function prototypes
void execute_builtin(char **args);
void execute_program(char **args);
char *get_program_path(char *program);

int main() {
    char input[MAX_INPUT_SIZE];
    char **args;  // Array of pointers to store command arguments

    while (1) {
        printf("shell> ");
        fflush(stdout);

        // Read user input and stop when Ctrl+D is pressed
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Remove newline character from the input
        input[strcspn(input, "\n")] = 0;

        // Use the parse function from helpers.c
        char *input_copy = strdup(input);
        args = parse(input_copy, " ");

        if (args != NULL && args[0] != NULL) {
            // Check if the command is a built-in function
            if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "help") == 0 ||
                strcmp(args[0], "pwd") == 0 || strcmp(args[0], "cd") == 0) {
                execute_builtin(args);
                if (strcmp(args[0], "exit") == 0) {
                    free(input_copy);
                    free(args);
                    break;
                }
            } else {
                execute_program(args);
            }
        }

        // Free the memory allocated by parse function and strdup
        free(input_copy);
        free(args);
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

void execute_program(char **args) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
    } else if (pid == 0) {
        // Child process
        
        char *program_path = get_program_path(args[0]);
        if (program_path == NULL) {
            fprintf(stderr, "Command not found: %s\n", args[0]);
            exit(1);
        }
        
        // Replace the child process with the new program
        if (execv(program_path, args) == -1) {
            perror("execv");
            exit(1);
        }
    } else {
        // Parent process waits for the child to complete
        waitpid(pid, NULL, 0);
    }
}

char *get_program_path(char *program) {
    // First, check if the program is specified with a full path
    if (strchr(program, '/') != NULL) {
        if (access(program, X_OK) == 0) {
            return strdup(program);
        }
        return NULL;
    }

    // If not, search in PATH
    char *path = getenv("PATH");
    if (path == NULL) {
        return NULL;
    }

    // Make a copy of PATH as strtok modifies the string
    char *path_copy = strdup(path);
    char *dir = strtok(path_copy, ":");
    
    while (dir != NULL) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, program);
        
        // Check if the program exists and is executable
        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return strdup(full_path);
        }
        
        dir = strtok(NULL, ":");
    }

    free(path_copy);
    return NULL;
}