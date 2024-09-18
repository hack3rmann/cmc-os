#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <inttypes.h>

enum
{
    ARGV_START_INDEX = 1,
    RADIX = 10,
};

int
main(int argc, char *argv[])
{
    uint64_t positive_sum = 0;
    uint64_t minus_negative_sum = 0;

    for (size_t i = ARGV_START_INDEX; nullptr != argv[i]; ++i) {
        char *end = nullptr;
        errno = 0;
        int64_t const parsed = strtoll(argv[i], &end, RADIX);

        if ('\0' != *end || 0 != errno || end == argv[i]) {
            fprintf(stderr, "invalid number '%s'\n", argv[i]);
            exit(EXIT_FAILURE);
        }

        if ((parsed > 0 && __builtin_add_overflow(positive_sum, (uint64_t) parsed, &positive_sum)) ||
            (parsed < 0 && __builtin_add_overflow(minus_negative_sum, (uint64_t) -parsed, &minus_negative_sum))) {

            fprintf(stderr, "integer overflow\n");
            exit(EXIT_FAILURE);
        }
    }

    if (0 == positive_sum && 0 == minus_negative_sum) {
        printf("0\n0\n");
        return 0;
    }

    printf("%" PRIu64 "\n-%" PRIu64 "\n", positive_sum, minus_negative_sum);
}
