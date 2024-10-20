#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

typedef int FileDesc;

enum
{
    SYSCALL_FAILURE = -1,
    FILE_ARG_INDEX = 1,
};

FileDesc
file_open(char const *file_name, mode_t flags)
{
    auto const file_desc = open(file_name, flags);

    if (SYSCALL_FAILURE == file_desc) {
        fprintf(stderr, "failed to open a file '%s': %s\n", file_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return file_desc;
}

void
file_close(FileDesc file_desc)
{
    if (SYSCALL_FAILURE == close(file_desc)) {
        fprintf(stderr, "failed to close a file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

ssize_t
file_read(FileDesc file_desc, void *buf, size_t count)
{
    auto const result = read(file_desc, buf, count);

    if (SYSCALL_FAILURE == result) {
        fprintf(stderr, "failed to read from file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return result;
}

off_t
file_seek(FileDesc file_desc, off_t offset, int whence)
{
    auto const result = lseek(file_desc, offset, whence);

    if (SYSCALL_FAILURE == result) {
        fprintf(stderr, "`lseek` failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return result;
}

typedef struct Node
{
    int32_t key;
    int32_t left_idx;
    int32_t right_idx;
} Node;

void
print_tree_recursive(FileDesc file_desc, off_t offset)
{
    file_seek(file_desc, offset, SEEK_SET);

    auto cur = (Node){};

    if (sizeof(cur) == file_read(file_desc, &cur, sizeof(cur))) {
        return;
    }

    if (0 != cur.right_idx) {
        print_tree_recursive(file_desc, cur.right_idx * sizeof(cur));
    }

    printf("%d ", cur.key);

    if (0 != cur.left_idx) {
        print_tree_recursive(file_desc, cur.left_idx * sizeof(cur));
    }
}

void
print_tree(FileDesc file_desc)
{
    print_tree_recursive(file_desc, 0);
}

int
main(int argc, char **argv)
{
    auto const file_desc = file_open(argv[FILE_ARG_INDEX], O_RDONLY);

    print_tree(file_desc);

    putchar('\n');

    file_close(file_desc);
}
