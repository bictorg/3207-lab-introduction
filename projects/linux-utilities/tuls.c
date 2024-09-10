#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

void list_directory(const char *path) {
    struct dirent **namelist;
    int n = scandir(path, &namelist, NULL, alphasort);
    
    if (n == -1) {
        printf("tuls: cannot open directory\n");
        exit(1);
    }

    if (strcmp(path, ".") != 0) {
        printf("[[%s]]\n", path);
    }
    
    for (int i = 0; i < n; i++) {
        struct stat st;
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, namelist[i]->d_name);
        
        if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            printf("  ->%s\n", namelist[i]->d_name);
        } else {
            printf("  %s\n", namelist[i]->d_name);
        }
        free(namelist[i]);
    }
    free(namelist);
    
    if (strcmp(path, ".") != 0) {
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        printf("Usage: %s [directory]\n", argv[0]);
        return 1;
    }

    if (argc == 2) {
        list_directory(argv[1]);
        list_directory(".");
    } else {
        list_directory(".");
    }

    return 0;
}

