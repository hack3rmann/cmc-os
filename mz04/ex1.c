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

    if (1 != scanf_result || 0 != errno) {
        fprintf(stderr, "failed to read a number from stdin\n");
        exit(EXIT_FAILURE);
    }

    if (result < 0 || result > MAX_24BIT) {
        fprintf(stderr, "invalid 24-bit number %d\n", result);
        exit(EXIT_FAILURE);
    }

    return (uint32_t) result;
}

enum
{
    WORD_SIZE_BYTES = 4,
};

void
write_word_exact(int file_desc, uint8_t const bytes[4])
{
    size_t n_written = 0;

    while (n_written < WORD_SIZE_BYTES) {
        auto const result = write(file_desc, bytes, WORD_SIZE_BYTES);

        if (-1 == result) {
            fprintf(stderr, "failed to write to file descriptor %d: %s\n", file_desc, strerror(errno));
            exit(EXIT_FAILURE);
        }

        n_written += (size_t) result;
    }
}

enum
{
    PERMISSIONS = 0600,
};

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "2 arguments required\n");
        exit(EXIT_FAILURE);
    }

    auto const filename = argv[1];
    auto const filename_len = strlen(filename);

    if (0 == filename_len) {
        fprintf(stderr, "invalid empty filename\n");
        exit(EXIT_FAILURE);
    }

    auto const file_desc = creat(filename, PERMISSIONS);

    if (-1 == file_desc) {
        fprintf(stderr, "failed to open file '%s': %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (uint32_t cur_number = read_24bit_from_stdin(); EOF_24BIT != cur_number; cur_number = read_24bit_from_stdin()) {
        uint8_t const bytes[] = {
            cur_number >> 20,
            255 & (cur_number >> 12),
            15 & (cur_number >> 8),
            255 & cur_number,
        };

        write_word_exact(file_desc, bytes);
    }

    if (0 != close(file_desc)) {
        fprintf(stderr, "failed to close file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}
