#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

enum
{
    BASE_TEN_RADIX = 10,
    PRINT_FILES_MAX_DEPTH = 4,
    CLOSE_DIR_FAILURE = -1,
    SYSCALL_FAILURE = -1,
    START_PATH_ARG_INDEX = 1,
    SIZE_ARG_INDEX = 2,
    REQUIRED_N_ARGS = SIZE_ARG_INDEX + 1,
};

void
print_files_recursive(char const *path, size_t first_path_size, size_t size, size_t depth)
{
    if (depth >= PRINT_FILES_MAX_DEPTH) {
        return;
    }

    auto const dir = opendir(path);

    if (nullptr == dir) {
        return;
    }

    for (auto entry = readdir(dir); nullptr != entry; entry = readdir(dir)) {
        if (0 == strcmp(entry->d_name, ".") || 0 == strcmp(entry->d_name, "..")) {
            continue;
        }

        auto absolute_path = (char *) nullptr;
        asprintf(&absolute_path, "%s/%s", path, entry->d_name);

        if (nullptr == absolute_path) {
            exit(EXIT_FAILURE);
        }

        auto file_stat = (struct stat){};

        if (SYSCALL_FAILURE == lstat(absolute_path, &file_stat)) {
            fprintf(stderr, "failed to stat directory '%s': %s\n", absolute_path, strerror(errno));
            free(absolute_path);
            exit(EXIT_FAILURE);
        }

        if (S_ISDIR(file_stat.st_mode)) {
            print_files_recursive(absolute_path, depth + 1, size, first_path_size);
        } else if ((size_t) file_stat.st_size <= size && S_ISREG(file_stat.st_mode) &&
                   0 == access(absolute_path, R_OK)) {
            printf("%s\n", absolute_path + first_path_size);
        }

        free(absolute_path);
    }

    if (CLOSE_DIR_FAILURE == closedir(dir)) {
        fprintf(stderr, "failed to close directory '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void
print_files(char const *path, size_t first_path_size, size_t size)
{
    print_files_recursive(path, first_path_size, size, 0);
}

size_t
size_parse(char const *source)
{
    char *parse_end = nullptr;
    errno = 0;
    auto const result = strtoll(source, &parse_end, BASE_TEN_RADIX);

    if (0 != errno || '\0' != *parse_end || parse_end == source || result < 0) {
        fprintf(stderr, "invalid size_t integer '%s'\n", source);
        exit(EXIT_FAILURE);
    }

    return (size_t) result;
}

int
main(int argc, char *argv[])
{
    if (argc != REQUIRED_N_ARGS) {
        fprintf(stderr, "%d args required\n", REQUIRED_N_ARGS);
        exit(EXIT_FAILURE);
    }

    auto const start_path = argv[START_PATH_ARG_INDEX];
    auto const start_path_size = strlen(start_path) + 1;
    auto const size = size_parse(argv[SIZE_ARG_INDEX]);

    print_files(start_path, start_path_size, size);
}
