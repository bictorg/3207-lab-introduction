#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "helpers.h"
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64
#define MAX_PIPES 10
#define MAX_BACKGROUND_JOBS 64

// background job PIDs
pid_t background_jobs[MAX_BACKGROUND_JOBS];
int num_background_jobs = 0;

// Function prototypes
void execute_builtin(char **args);
void execute_program(char **args, int input_fd, int output_fd, int run_in_background);
char *get_program_path(char *program);
void parse_and_execute(char *input);
void execute_pipeline(char ***commands, int num_commands);
void add_background_job(pid_t pid);
void wait_for_background_jobs();

int main() {
    char input[MAX_INPUT_SIZE];

    while (1) {
        printf("shell> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = 0;
        parse_and_execute(input);
    }

    return 0;
}

void parse_and_execute(char *input) {
    char *commands[MAX_PIPES];
    int num_commands = 0;
    char *token = strtok(input, "|");
    
    while (token != NULL && num_commands < MAX_PIPES) {
        commands[num_commands++] = token;
        token = strtok(NULL, "|");
    }

    char ***parsed_commands = malloc(num_commands * sizeof(char **));
    for (int i = 0; i < num_commands; i++) {
        parsed_commands[i] = malloc(MAX_ARGS * sizeof(char *));
        int arg_count = 0;
        token = strtok(commands[i], " ");
        while (token != NULL && arg_count < MAX_ARGS - 1) {
            parsed_commands[i][arg_count++] = token;
            token = strtok(NULL, " ");
        }
        parsed_commands[i][arg_count] = NULL;
    }

    execute_pipeline(parsed_commands, num_commands);

    // Free all allocated memory
    for (int i = 0; i < num_commands; i++) {
        free(parsed_commands[i]);
    }
    free(parsed_commands);
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

// @TODO: use input_fd, output_fd, run_in_background
void execute_program(char **args, int input_fd, int output_fd, int run_in_background) {
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