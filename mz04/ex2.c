#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

enum
{
    MIN_N_ARGS = 3,
    RW_USER_GROUP_PERMISSIONS = 0660,
    BASE_TEN_RADIX = 10,
    MIN_N_VALUES = 1,
    FILE_NAME_INDEX = 1,
    N_VALUES_INDEX = 2,
    SYSCALL_FAILURE = -1,
};

void
print_lseek_failed_message(void)
{
    fprintf(stderr, "`lseek` failed: %s\n", strerror(errno));
}

int
main(int argc, char **argv)
{
    if (argc < MIN_N_ARGS) {
        exit(EXIT_FAILURE);
    }

    auto const file_name = argv[FILE_NAME_INDEX];
    errno = 0;
    char *parse_end = nullptr;
    auto n_values = strtol(argv[N_VALUES_INDEX], &parse_end, BASE_TEN_RADIX);

    if (0 != errno || '\0' != *parse_end || parse_end == argv[N_VALUES_INDEX] || n_values != (int) n_values) {
        fprintf(stderr, "invalid number of values '%s'\n", argv[N_VALUES_INDEX]);
        exit(EXIT_FAILURE);
    }

    if (n_values < MIN_N_VALUES) {
        return 0;
    }

    errno = 0;
    auto const file_desc = open(file_name, O_RDWR, RW_USER_GROUP_PERMISSIONS);

    if (SYSCALL_FAILURE == file_desc) {
        fprintf(stderr, "failed to open a file '%s': %s\n", file_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    errno = 0;
    auto file_size = lseek(file_desc, 0, SEEK_END);

    if (SYSCALL_FAILURE == file_size) {
        print_lseek_failed_message();
        exit(EXIT_FAILURE);
    }

    if (file_size / sizeof(double) < n_values) {
        n_values = file_size / sizeof(double);
    }

    errno = 0;

    if (SYSCALL_FAILURE == lseek(file_desc, 0, SEEK_SET)) {
        print_lseek_failed_message();
        exit(EXIT_FAILURE);
    }

    auto prev = 0.0;
    auto cur = 0.0;

    errno = 0;

    if (SYSCALL_FAILURE == read(file_desc, &prev, sizeof(prev))) {
        fprintf(stderr, "failed to read from file '%s': %s\n", file_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (size_t i = 1; i < (size_t) n_values; i++) {
        errno = 0;

        if (SYSCALL_FAILURE == read(file_desc, &cur, sizeof(cur))) {
            fprintf(stderr, "Error code: %d\n", errno);
            exit(EXIT_FAILURE);
        }

        auto const new_cur = cur - prev;

        errno = 0;

        if (SYSCALL_FAILURE == lseek(file_desc, -sizeof(new_cur), SEEK_CUR)) {
            fprintf(stderr, "Error code: %d\n", errno);
            exit(EXIT_FAILURE);
        }

        errno = 0;

        if (SYSCALL_FAILURE == write(file_desc, &new_cur, sizeof(new_cur))) {
            fprintf(stderr, "Error code: %d\n", errno);
            exit(EXIT_FAILURE);
        }

        prev = new_cur;
    }

    if (SYSCALL_FAILURE == close(file_desc)) {
        fprintf(stderr, "failed to close a file '%s': %s\n", file_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
}
