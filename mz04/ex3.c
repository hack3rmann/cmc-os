#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

typedef int FileDesc;
typedef long long llong;

enum
{
    MIN_N_ARGS = 2,
    FILE_NAME_INDEX = 1,
    SYSCALL_FAILURE = -1,
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

ssize_t
file_write(FileDesc file_desc, void const *buf, size_t count)
{
    auto const result = write(file_desc, buf, count);

    if (SYSCALL_FAILURE == result) {
        fprintf(stderr, "failed to write to file: %s\n", strerror(errno));
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

int
main(int argc, char **argv)
{
    if (argc < MIN_N_ARGS) {
        fprintf(stderr, "%d arguments required\n", MIN_N_ARGS);
        exit(EXIT_FAILURE);
    }

    auto const file_name = argv[FILE_NAME_INDEX];
    auto const file_desc = file_open(file_name, O_RDWR);

    llong min = 0;
    off_t min_offset = 0;

    if (sizeof(min) != file_read(file_desc, &min, sizeof(min))) {
        return 0;
    }

    llong cur = 0;
    off_t cur_offset = 0;

    while (sizeof(cur) == file_read(file_desc, &cur, sizeof(cur))) {
        cur_offset += sizeof(cur);

        if (min > cur) {
            min = cur;
            min_offset = cur_offset;
        }
    }

    if (min > LLONG_MIN) {
        file_seek(file_desc, min_offset, SEEK_SET);

        auto const new_value = -min;
        file_write(file_desc, &new_value, sizeof(min));
    }

    file_close(file_desc);
}
