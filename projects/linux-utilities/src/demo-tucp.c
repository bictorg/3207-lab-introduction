#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h>

#define MAX_PATH 1024

void create_test_file(const char *path, const char *content) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("Error creating test file");
        exit(1);
    }
    fprintf(file, "%s\n", content);
    fclose(file);
}

void run_tucp(int argc, char *argv[], const char *exe_path) {
    char command[MAX_PATH * 2];
    snprintf(command, sizeof(command), "%s", exe_path);
    for (int i = 1; i < argc; i++) {
        strcat(command, " ");
        strcat(command, argv[i]);
    }
    system(command);
}

void cat_file(const char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        return;
    }
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    char exe_path[MAX_PATH];
    char *dir_path;

    if (readlink("/proc/self/exe", exe_path, sizeof(exe_path)) == -1) {
        perror("Error getting executable path");
        return 1;
    }

    dir_path = dirname(exe_path);
    chdir(dir_path);

    char tucp_path[MAX_PATH];
    snprintf(tucp_path, sizeof(tucp_path), "%s/tucp", dir_path);

    char compile_command[MAX_PATH * 2];
    snprintf(compile_command, sizeof(compile_command), "gcc -o %s %s/tucp.c", tucp_path, dir_path);
    system(compile_command);

    char test_src[MAX_PATH], test_dest[MAX_PATH];
    snprintf(test_src, sizeof(test_src), "%s/test_src", dir_path);
    snprintf(test_dest, sizeof(test_dest), "%s/test_dest", dir_path);

    mkdir(test_src, 0755);
    mkdir(test_src, 0755);
    snprintf(test_src, sizeof(test_src), "%s/test_src/subdir", dir_path);
    mkdir(test_src, 0755);
    snprintf(test_dest, sizeof(test_dest), "%s/test_dest", dir_path);
    mkdir(test_dest, 0755);

    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s/test_src/file1.txt", dir_path);
    create_test_file(file_path, "Hello, World!");
    snprintf(file_path, sizeof(file_path), "%s/test_src/file2.txt", dir_path);
    create_test_file(file_path, "This is a test.");
    snprintf(file_path, sizeof(file_path), "%s/test_src/subdir/file3.txt", dir_path);
    create_test_file(file_path, "Subdirectory file");

    // Single file copy
    printf("Single file copy:\n");
    char *single_copy_args[] = {"tucp", "test_src/file1.txt", "test_dest/file1_copy.txt"};
    run_tucp(3, single_copy_args, tucp_path);
    printf("Contents of test_dest/file1_copy.txt:\n");
    snprintf(file_path, sizeof(file_path), "%s/test_dest/file1_copy.txt", dir_path);
    cat_file(file_path);
    printf("\n");

    // Multi file copy
    printf("Multiple file copy:\n");
    char *multi_copy_args[] = {"tucp", "test_src/file1.txt", "test_src/file2.txt", "test_dest"};
    run_tucp(4, multi_copy_args, tucp_path);

    // Logging
    printf("Results:\n");
    printf("Contents of multi copied files:\n");
    printf("test_dest/file1.txt:\n");
    snprintf(file_path, sizeof(file_path), "%s/test_dest/file1.txt", dir_path);
    cat_file(file_path);
    printf("\ntest_dest/file2.txt:\n");
    snprintf(file_path, sizeof(file_path), "%s/test_dest/file2.txt", dir_path);
    cat_file(file_path);
    printf("\n");

    // Cleanup ?
    char cleanup_choice;
    printf("Want to remove the test files? (y/n): ");
    scanf(" %c", &cleanup_choice);
    if (cleanup_choice == 'y' || cleanup_choice == 'Y') {
        printf("Cleaning up...\n");
        char cleanup_command[MAX_PATH * 2];
        snprintf(cleanup_command, sizeof(cleanup_command), "rm -rf %s/test_src %s/test_dest %s", dir_path, dir_path, tucp_path);
        system(cleanup_command);
    } else {
        printf("Skipping cleanup.\n");
    }

    return 0;
}
