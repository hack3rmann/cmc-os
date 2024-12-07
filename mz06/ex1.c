#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static char const exe_suffix[] = ".exe";

enum
{
    CLOSE_DIR_FAILURE = -1,
    SYSCALL_FAILURE = -1,
    EXE_SUFFIX_LEN = sizeof(exe_suffix) - 1,
    DIR_PATH_ARG_INDEX = 1,
    MIN_N_ARGS = DIR_PATH_ARG_INDEX + 1,
};

bool
contains_suffix(char const *src, size_t src_len, char const *suffix, size_t suffix_len)
{
    return src_len >= suffix_len && 0 == strcmp(src + src_len - suffix_len, suffix);
}

int
main(int argc, char *argv[])
{
    if (argc < MIN_N_ARGS) {
        fprintf(stderr, "%d args required\n", MIN_N_ARGS);
        exit(EXIT_FAILURE);
    }

    auto const dir_path = argv[DIR_PATH_ARG_INDEX];
    auto const dir = opendir(dir_path);

    if (nullptr == dir) {
        fprintf(stderr, "failed to open directory '%s': %s\n", dir_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    size_t n_files = 0;

    for (auto entry = readdir(dir); nullptr != entry; entry = readdir(dir)) {
        char absolute_path[PATH_MAX] = {};

        if (snprintf(absolute_path, sizeof(absolute_path), "%s/%s", dir_path, entry->d_name) >= PATH_MAX) {
            continue;
        }

        auto file_stat = (struct stat){};

        if (SYSCALL_FAILURE == stat(absolute_path, &file_stat)) {
            fprintf(stderr, "failed to stat '%s': %s\n", absolute_path, strerror(errno));
            exit(EXIT_FAILURE);
        }

        auto const entry_name_len = strlen(entry->d_name);
        auto const is_regular_executable = S_ISREG(file_stat.st_mode) && !access(absolute_path, X_OK);

        if (is_regular_executable && contains_suffix(entry->d_name, entry_name_len, exe_suffix, EXE_SUFFIX_LEN)) {
            n_files += 1;
        }
    }

    if (CLOSE_DIR_FAILURE == closedir(dir)) {
        fprintf(stderr, "failed to close directory '%s': %s\n", dir_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("%zu\n", n_files);
}
