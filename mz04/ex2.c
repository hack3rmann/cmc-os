#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef int FileDesc;

enum
{
    FILE_NAME_ARG_INDEX = 1,
    N_ELEMENTS_ARG_INDEX = FILE_NAME_ARG_INDEX + 1,
    MIN_N_ARGS = N_ELEMENTS_ARG_INDEX + 1,
    RW_USER_GROUP_PERMISSON = 0660,
    BASE_TEN_RADIX = 10,
    SYSCALL_FAIL_VALUE = -1,
};

double
double_read_exact(FileDesc file_desc)
{
    uint8_t result[sizeof(double)] = {};
    size_t n_remaining_bytes = sizeof(result);

    while (0 != n_remaining_bytes) {
        auto const n_read = read(file_desc, result + sizeof(result) - n_remaining_bytes, n_remaining_bytes);

        if (SYSCALL_FAIL_VALUE == n_read) {
            fprintf(stderr, "failed to read from file #%d: %s\n", file_desc, strerror(errno));
            exit(EXIT_FAILURE);
        }

        n_remaining_bytes -= n_read;
    }

    return *(double *) result;
}

void
double_write_exact(FileDesc file_desc, double value)
{
    size_t n_remaining_bytes = sizeof(value);
    auto const value_bytes = (uint8_t *) &value;

    while (0 != n_remaining_bytes) {
        auto const n_wrote = write(file_desc, value_bytes + sizeof(value) - n_remaining_bytes, n_remaining_bytes);

        if (SYSCALL_FAIL_VALUE == n_wrote) {
            fprintf(stderr, "filed to write to file #%d: %s\n", file_desc, strerror(errno));
            exit(EXIT_FAILURE);
        }

        n_remaining_bytes -= n_wrote;
    }
}

int
main(int argc, char *argv[])
{
    if (argc < MIN_N_ARGS) {
        fprintf(stderr, "minimum %d arguments required, but got %d\n", MIN_N_ARGS, argc);
        exit(EXIT_FAILURE);
    }

    errno = 0;
    char *parse_end = nullptr;
    auto const n_elements = strtol(argv[N_ELEMENTS_ARG_INDEX], &parse_end, BASE_TEN_RADIX);

    if ('\0' != *parse_end || 0 != errno || parse_end == argv[N_ELEMENTS_ARG_INDEX]) {
        fprintf(stderr, "invalid element count '%s'\n", argv[N_ELEMENTS_ARG_INDEX]);
        exit(EXIT_FAILURE);
    }

    if (n_elements < 1) {
        exit(EXIT_SUCCESS);
    }

    auto const file_name = argv[FILE_NAME_ARG_INDEX];
    auto const file_desc = open(file_name, O_RDWR, RW_USER_GROUP_PERMISSON);

    if (SYSCALL_FAIL_VALUE == file_desc) {
        fprintf(stderr, "failed to open file '%s': %s\n", file_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    auto const file_size = lseek(file_desc, 0, SEEK_END);

    if (SYSCALL_FAIL_VALUE == file_size) {
        fprintf(stderr, "`lseek` failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    auto n_elements_to_proccess = (size_t) n_elements;

    if (0 != file_size % sizeof(double) || file_size < n_elements * sizeof(double)) {
        n_elements_to_proccess = (size_t) file_size / sizeof(double);
    }

    if (SYSCALL_FAIL_VALUE == lseek(file_desc, 0, SEEK_SET)) {
        fprintf(stderr, "`lseek` failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    auto prev = double_read_exact(file_desc);

    for (size_t i = 1; i < n_elements_to_proccess; i++) {
        auto const cur = double_read_exact(file_desc);
        auto const next = cur - prev;

        if (SYSCALL_FAIL_VALUE == lseek(file_desc, -sizeof(next), SEEK_CUR)) {
            fprintf(stderr, "`lseek` failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        double_write_exact(file_desc, next);

        prev = next;
    }

    if (SYSCALL_FAIL_VALUE == close(file_desc)) {
        fprintf(stderr, "failed to close a file '%s': %s\n", file_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
}
