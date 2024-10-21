#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

enum
{
    PATH_ARG_INDEX = 1,
    N_REQUIRED_ARGS = PATH_ARG_INDEX + 1,
    SYSCALL_FAILURE = -1,
};

int
main(int argc, char *argv[])
{
    if (argc != N_REQUIRED_ARGS) {
        fprintf(stderr, "%d args required\n", N_REQUIRED_ARGS);
        exit(EXIT_FAILURE);
    }

    while (true) {
        char *path = nullptr;
        asprintf(&path, "%s", argv[PATH_ARG_INDEX]);

        if (nullptr == path) {
            exit(EXIT_FAILURE);
        }

        auto file_stat = (struct stat){};

        if (SYSCALL_FAILURE == lstat(path, &file_stat)) {
            fprintf(stderr, "failed to stat file '%s': %s\n", path, strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (S_ISREG(file_stat.st_mode) && !access(path, R_OK) && (file_stat.st_mode & S_ISVTX)) {
            auto const file = fopen(path, "r");

            free(path);
            fscanf(file, "%ms", &path);

            if (nullptr == path) {
                exit(EXIT_FAILURE);
            }

            fclose(file);
        } else if (S_ISDIR(file_stat.st_mode)) {
            auto const dir = opendir(path);

            if (nullptr == dir) {
                exit(EXIT_FAILURE);
            }

            size_t total_size = 0;

            for (auto entry = readdir(dir); nullptr != entry; entry = readdir(dir)) {
                char *absolute_path = nullptr;
                asprintf(&absolute_path, "%s/%s", path, entry->d_name);

                auto file_stat = (struct stat){};

                if (SYSCALL_FAILURE == stat(absolute_path, &file_stat)) {
                    free(absolute_path);
                    continue;
                }

                if (!access(absolute_path, X_OK) && S_ISREG(file_stat.st_mode)) {
                    total_size += file_stat.st_size;
                }

                free(absolute_path);
            }

            closedir(dir);

            printf("%zu\n", total_size);

            free(path);
            break;
        } else {
            auto file_stat = (struct stat){};
            if (SYSCALL_FAILURE == lstat(path, &file_stat)) {
                exit(EXIT_FAILURE);
            }

            printf("%zu\n", file_stat.st_size);

            free(path);
            break;
        }

        free(path);
    }
}
