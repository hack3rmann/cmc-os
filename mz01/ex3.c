#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

uint16_t *
calculate_division_table(uint16_t size)
{
    uint16_t *result = calloc((size_t) size * size, sizeof(*result));

    if (!result) {
        fprintf(stderr, "failed to allocate more memory\n");
        exit(EXIT_FAILURE);
    }

    for (uint16_t i = 1; i < size; ++i) {
        for (uint16_t j = 1; j < size; ++j) {
            uint16_t const product = i * j % size;

            result[product + size * i] = j;
        }
    }

    return result;
}

bool
is_prime(uint32_t value)
{
    for (uint32_t i = 2; i * i <= value; ++i) {
        if (value % i == 0) {
            return false;
        }
    }

    return true;
}

enum
{
    MAX_TABLE_SIZE = 2000,
};

int
main(void)
{
    ssize_t table_size;

    errno = 0;

    if (1 != scanf("%zd", &table_size)) {
        fprintf(stderr, "failed to read from stdin\n");
        exit(EXIT_FAILURE);
    }

    if (0 != errno) {
        fprintf(stderr, "conversion error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (table_size >= MAX_TABLE_SIZE || table_size <= 1) {
        fprintf(stderr, "invalid table size: should be less than 2000 and greater than 1\n");
        exit(EXIT_FAILURE);
    }

    if (!is_prime(table_size)) {
        fprintf(stderr, "invalid table size: should be prime\n");
        exit(EXIT_FAILURE);
    }

    uint16_t *table = calculate_division_table(table_size);

    for (size_t i = 0; i < (size_t) table_size; ++i) {
        for (size_t j = 1; j < (size_t) table_size; ++j) {
            printf("%u ", table[i + table_size * j]);
        }

        putchar('\n');
    }

    free(table);
}
