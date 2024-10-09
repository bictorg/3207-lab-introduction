#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
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
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    } else if (strcmp(args[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  exit - Exit the shell\n");
        printf("  help - Display this help message\n");
        printf("  pwd - Print current working directory\n");
        printf("  cd <directory> - Change current working directory\n");
        printf("  wait - Wait for all background processes to finish\n");
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
        // Print current directory after changing
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
    } else if (strcmp(args[0], "wait") == 0) {
        wait_for_background_jobs();
    }
}

void execute_program(char **args, int input_fd, int output_fd, int run_in_background) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
    } else if (pid == 0) {
        // Child process
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != STDOUT_FILENO) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        
        char *program_path = get_program_path(args[0]);
        if (program_path == NULL) {
            fprintf(stderr, "Command not found: %s\n", args[0]);
            exit(1);
        }
        
        if (execv(program_path, args) == -1) {
            perror("execv");
            exit(1);
        }
    } else {
        // Parent process
        if (input_fd != STDIN_FILENO) close(input_fd);
        if (output_fd != STDOUT_FILENO) close(output_fd);
        
        if (run_in_background) {
            printf("[%d] %d\n", num_background_jobs + 1, pid);
            add_background_job(pid);
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
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

void execute_pipeline(char ***commands, int num_commands) {
    int pipes[MAX_PIPES][2];
    
    // Create pipes for all but the last command
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return;
        }
    }

    for (int i = 0; i < num_commands; i++) {
        int input_fd = (i == 0) ? STDIN_FILENO : pipes[i-1][0];
        int output_fd = (i == num_commands - 1) ? STDOUT_FILENO : pipes[i][1];

        // Handle input redirection
        for (int j = 0; commands[i][j] != NULL; j++) {
            if (strcmp(commands[i][j], "<") == 0 && commands[i][j+1] != NULL) {
                int fd = open(commands[i][j+1], O_RDONLY);
                if (fd == -1) {
                    perror("open");
                    return;
                }
                input_fd = fd;
                // Remove redirection symbols / filename from arguments
                for (int k = j; commands[i][k] != NULL; k++) {
                    commands[i][k] = commands[i][k+2];
                }
                break;
            }
        }

        // Handle output redirection
        for (int j = 0; commands[i][j] != NULL; j++) {
            if (strcmp(commands[i][j], ">") == 0 && commands[i][j+1] != NULL) {
                int fd = open(commands[i][j+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("open");
                    return;
                }
                output_fd = fd;
                // Remove redirection symbols / filename from arguments
                commands[i][j] = NULL;
                break;
            }
        }

        // Check if cmd should run in the background
        int run_in_background = 0;
        int last_arg = 0;
        while (commands[i][last_arg] != NULL) last_arg++;
        if (last_arg > 0 && strcmp(commands[i][last_arg - 1], "&") == 0) {
            run_in_background = 1;
            commands[i][last_arg - 1] = NULL;
        }

        // Execute built-in commands or external programs
        if (strcmp(commands[i][0], "exit") == 0 || strcmp(commands[i][0], "help") == 0 ||
            strcmp(commands[i][0], "pwd") == 0 || strcmp(commands[i][0], "cd") == 0 ||
            strcmp(commands[i][0], "wait") == 0) {
            execute_builtin(commands[i]);
        } else {
            execute_program(commands[i], input_fd, output_fd, run_in_background);
        }
    }

    // Close all pipe file descriptors in the parent
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

void add_background_job(pid_t pid) {
    if (num_background_jobs < MAX_BACKGROUND_JOBS) {
        background_jobs[num_background_jobs++] = pid;
    } else {
        fprintf(stderr, "Maximum number of background jobs reached\n");
    }
}

void wait_for_background_jobs() {
    for (int i = 0; i < num_background_jobs; i++) {
        int status;
        pid_t pid = waitpid(background_jobs[i], &status, 0);
        if (pid > 0) {
            printf("[%d] Done\n", i + 1);
        }
    }
    num_background_jobs = 0;
}