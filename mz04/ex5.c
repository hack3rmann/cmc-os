#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

typedef int FileDesc;

enum
{
    ARG_INPUT_FILE_INDEX = 1,
    ARG_OUTUT_FILE_INDEX = 2,
    MIN_N_ARGS = 4,
    BASE_TEN_RADIX = 10,
    N_BYTE_BITS = CHAR_BIT,
    RW_USER_PERMMISSION = 0600,
    ARG_MOD_INDEX = 3,
    SYSCALL_FAILURE = -1,
};

int32_t
i32_parse(char const *src)
{
    errno = 0;
    char *parse_end = nullptr;
    auto const parsed = strtol(src, &parse_end, BASE_TEN_RADIX);

    if (0 != errno || '\0' != *parse_end || parse_end == src || parsed != (int32_t) parsed) {
        fprintf(stderr, "failed to parse '%s' as 32-bit signed integer\n", src);
        exit(EXIT_FAILURE);
    }

    return (int32_t) parsed;
}

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

FileDesc
file_create(char const *file_name, mode_t permissions)
{
    auto const file_desc = creat(file_name, permissions);

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

int
main(int argc, char **argv)
{
    if (argc < MIN_N_ARGS) {
        fprintf(stderr, "%d arguments required\n", MIN_N_ARGS);
        exit(EXIT_FAILURE);
    }

    auto const mod = i32_parse(argv[ARG_MOD_INDEX]);

    auto const input_file_name = argv[ARG_INPUT_FILE_INDEX];
    auto const output_file_name = argv[ARG_OUTUT_FILE_INDEX];

    auto const input_file_desc = file_open(input_file_name, O_RDONLY);
    auto const output_file_desc = file_create(output_file_name, RW_USER_PERMMISSION);

    uint64_t cur_num = 0, sum = 0;
    uint8_t cur_byte = 0;

    while (sizeof(cur_byte) == file_read(input_file_desc, &cur_byte, sizeof(cur_byte))) {
        for (size_t i = 0; i < N_BYTE_BITS; i++) {
            cur_num += 1;
            sum += cur_num * cur_num;

            if (1 & cur_byte) {
                auto const result = (int32_t) (sum % mod);
                write(output_file_desc, &result, sizeof(result));
            }

            cur_byte >>= 1;
        }
    }

    file_close(output_file_desc);
    file_close(input_file_desc);
}
