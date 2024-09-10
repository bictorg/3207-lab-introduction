#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

int copy_file(const char *src, const char *dest) {
    int src_fd, dest_fd;
    ssize_t bytes_read, bytes_written;
    char buffer[BUFFER_SIZE];

    src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        perror("Error opening source file");
        return -1;
    }

    dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        perror("Error opening destination file");
        close(src_fd);
        return -1;
    }

    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Write error");
            close(src_fd);
            close(dest_fd);
            return -1;
        }
    }

    if (bytes_read == -1) {
        perror("Read error");
        close(src_fd);
        close(dest_fd);
        return -1;
    }

    close(src_fd);
    close(dest_fd);
    return 0;
}

int is_directory(const char *path) {
  struct stat file_info;

  return (stat(path, &file_info) == 0 && S_ISDIR(file_info.st_mode));
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <source_file(s)> <destination>\n", argv[0]);
        return 1;
    }

    const char *destination = argv[argc - 1];
    int is_dest_dir = is_directory(destination);

    if (argc == 3 && !is_dest_dir) {
        // Case 1: Single file copy
        if (copy_file(argv[1], argv[2]) != 0) {
            fprintf(stderr, "Failed to copy %s to %s\n", argv[1], argv[2]);
            return 1;
        }
    } else {
        // Case 2 & 3: Copy to directory
        if (!is_dest_dir) {
            fprintf(stderr, "Destination must be a directory when copying multiple files\n");
            return 1;
        }

        for (int i = 1; i < argc - 1; i++) {
            char dest_path[PATH_MAX];
            snprintf(dest_path, sizeof(dest_path), "%s/%s", destination, basename(argv[i]));
            
            if (copy_file(argv[i], dest_path) != 0) {
                fprintf(stderr, "Failed to copy %s to %s\n", argv[i], dest_path);
                return 1;
            }
        }
    }

    return 0;
}
