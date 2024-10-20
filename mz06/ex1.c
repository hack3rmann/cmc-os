#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>

enum
{
    SYSCALL_FAILURE = -1,
};

void
tree_recursive(char const *dir_path, bool show_hidden, size_t depth)
{
    auto const dir = opendir(dir_path);

    if (nullptr == dir) {
        fprintf(stderr, "failed to open directory '%s': %s\n", dir_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (struct dirent *entry = readdir(dir); nullptr != entry; entry = readdir(dir)) {
        char *entry_path = nullptr;

        if (0 == strcmp(entry->d_name, ".") || 0 == strcmp(entry->d_name, "..")) {
            free(entry_path);
            continue;
        }

        if (!show_hidden && entry->d_name[0] == '.') {
            free(entry_path);
            continue;
        }

        asprintf(&entry_path, "%s/%s", dir_path, entry->d_name);
        struct stat entry_stat;

        if (SYSCALL_FAILURE == lstat(entry_path, &entry_stat)) {
            fprintf(stderr, "failed to stat file '%s': %s\n", entry_path, strerror(errno));
            free(entry_path);
            continue;
        }

        for (size_t i = 0; i < 2 * depth; ++i) {
            putchar(' ');
        }

        printf("%s\n", entry->d_name);

        if (S_ISDIR(entry_stat.st_mode)) {
            tree_recursive(entry_path, show_hidden, depth + 1);
        }

        free(entry_path);
    }

    closedir(dir);
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "2 args expected\n");
        exit(EXIT_FAILURE);
    }

    tree_recursive(argv[1], false, 0);
}
