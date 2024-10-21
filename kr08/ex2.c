#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <inttypes.h>

enum
{
    BASE_TEN_RADIX = 10,
};

int64_t
i64_parse(char const *src)
{
    char *parse_end = nullptr;
    int64_t const result = strtoll(src, &parse_end, BASE_TEN_RADIX);
    errno = 0;

    if (0 != errno || '\0' != *parse_end || src == parse_end) {
        fprintf(stderr, "invalid integer '%s'\n", src);
        exit(EXIT_FAILURE);
    }

    return result;
}

int
main(int argc, char *argv[])
{
    if (argc % 2 != 1) {
        exit(EXIT_FAILURE);
    }

    uint64_t unsigned_result = 0;
    size_t total_n_bits = 0;

    for (size_t i = argc - 1; i > 0; i -= 2) {
        auto const bit = (uint64_t) i64_parse(argv[i - 1]);
        auto const n_bits = (uint64_t) i64_parse(argv[i]);

        if (bit > 1) {
            fprintf(stderr, "invalid bit\n");
            exit(EXIT_FAILURE);
        }

        if (n_bits < 1 || 64 < n_bits) {
            fprintf(stderr, "invalid bit count\n");
            exit(EXIT_FAILURE);
        }

        total_n_bits += n_bits;

        for (size_t j = 0; j < n_bits; ++j) {
            unsigned_result <<= 1;
            unsigned_result |= bit;
        }
    }

    if (total_n_bits > 64) {
        fprintf(stderr, "too many bits\n");
        exit(EXIT_FAILURE);
    }

    auto const result = (int64_t) unsigned_result;

    printf("%" PRIi64 "\n", result);
}
