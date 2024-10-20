#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

enum
{
    SYSCALL_FAILURE = -1,
    FILE_NAMES_ARGS_START = 1,
};

int
string_ptr_cmp(const void *a, const void *b)
{
    auto const a_str = (char **) a;
    auto const b_str = (char **) b;

    return strcmp(*a_str, *b_str);
}

int
main(int argc, char *argv[])
{
    size_t const file_paths_cap = (size_t) (argc - 1);
    size_t file_paths_len = 0;
    char **file_paths = calloc(file_paths_cap, sizeof(*file_paths));

    if (nullptr == file_paths) {
        fprintf(stderr, "failed to allocate memory: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (size_t i = FILE_NAMES_ARGS_START; i < (size_t) argc; ++i) {
        auto const cur_file_name = argv[i];
        auto cur_file_stat = (struct stat){};
        auto other_file_stat = (struct stat){};
        auto is_unique = true;

        if (SYSCALL_FAILURE == stat(cur_file_name, &cur_file_stat)) {
            continue;
        }

        for (size_t j = 0; j < file_paths_len; ++j) {
            if (SYSCALL_FAILURE == stat(file_paths[j], &other_file_stat)) {
                continue;
            }

            if (cur_file_stat.st_ino == other_file_stat.st_ino && cur_file_stat.st_dev == other_file_stat.st_dev) {
                is_unique = false;

                if (strcmp(cur_file_name, file_paths[j]) > 0) {
                    file_paths[j] = cur_file_name;
                }

                break;
            }
        }

        if (is_unique) {
            file_paths[file_paths_len] = cur_file_name;
            file_paths_len += 1;
        }
    }

    qsort(file_paths, file_paths_len, sizeof(*file_paths), string_ptr_cmp);

    for (size_t i = 0; i < file_paths_len; ++i) {
        printf("%s\n", file_paths[i]);
    }

    free(file_paths);
}
