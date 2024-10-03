#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

enum
{
    EOF_24BIT = ~0u,
    MAX_24BIT = (1 << 24) - 1,
    SCANF_RESULT_1_ARGUMENT_READ = 1,
};

uint32_t
read_24bit_from_stdin(void)
{
    int result = 0;
    errno = 0;
    auto const scanf_result = scanf("%d", &result);

    if (EOF == scanf_result) {
        return EOF_24BIT;
    }

    if (SCANF_RESULT_1_ARGUMENT_READ != scanf_result || 0 != errno) {
        fprintf(stderr, "failed to read a number from stdin\n");
        exit(EXIT_FAILURE);
    }

    if (result < 0 || result > (int) MAX_24BIT) {
        fprintf(stderr, "invalid 24-bit number %d\n", result);
        exit(EXIT_FAILURE);
    }

    return (uint32_t) result;
}

enum
{
    WORD_SIZE_BYTES = 4,
    WRITE_ERROR_VALUE = -1,
    WORD_BYTE_SIZE = 4,
};

void
write_word_exact(int file_desc, uint8_t const bytes[WORD_BYTE_SIZE])
{
    size_t n_written = 0;

    while (n_written < WORD_SIZE_BYTES) {
        auto const result = write(file_desc, bytes, WORD_SIZE_BYTES);

        if (WRITE_ERROR_VALUE == result) {
            fprintf(stderr, "failed to write to file descriptor %d: %s\n", file_desc, strerror(errno));
            exit(EXIT_FAILURE);
        }

        n_written += (size_t) result;
    }
}

enum
{
    PERMISSIONS = 0600,
    MASK_4BIT = 15,
    MASK_8BIT = 255,
    FIRST_BYTE_SHIFT = 20,
    SECOND_BYTE_SHIFT = 12,
    THIRD_BYTE_SHIFT = 8,
    FOURTH_BYTE_SHIFT = 0,
    FIRST_BYTE_MASK = MASK_4BIT,
    SECOND_BYTE_MASK = MASK_8BIT,
    THIRD_BYTE_MASK = MASK_4BIT,
    FOURTH_BYTE_MASK = MASK_8BIT,
    MIN_N_ARGS = 2,
    FIRST_ARG_INDEX = MIN_N_ARGS - 1,
    CREAT_ERROR_RESULT = -1,
};

int
main(int argc, char *argv[])
{
    if (argc < MIN_N_ARGS) {
        fprintf(stderr, "%zu arguments required\n", (size_t) MIN_N_ARGS);
        exit(EXIT_FAILURE);
    }

    auto const filename = argv[FIRST_ARG_INDEX];
    auto const filename_len = strlen(filename);

    if (0 == filename_len) {
        fprintf(stderr, "invalid empty filename\n");
        exit(EXIT_FAILURE);
    }

    auto const file_desc = creat(filename, PERMISSIONS);

    if (CREAT_ERROR_RESULT == file_desc) {
        fprintf(stderr, "failed to open file '%s': %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (uint32_t cur_number = read_24bit_from_stdin(); EOF_24BIT != cur_number; cur_number = read_24bit_from_stdin()) {
        uint8_t const bytes[] = {
            FIRST_BYTE_MASK & (cur_number >> FIRST_BYTE_SHIFT),
            SECOND_BYTE_MASK & (cur_number >> SECOND_BYTE_SHIFT),
            THIRD_BYTE_MASK & (cur_number >> THIRD_BYTE_SHIFT),
            FOURTH_BYTE_MASK & (cur_number >> FOURTH_BYTE_SHIFT),
        };

        write_word_exact(file_desc, bytes);
    }

    if (0 != close(file_desc)) {
        fprintf(stderr, "failed to close file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}
