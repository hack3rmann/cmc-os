#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

uint64_t
factorial(uint8_t value)
{
    uint64_t result = 1;

    for (size_t i = 2; i <= value; ++i) {
        result *= i;
    }

    return result;
}

void
swap_bytes(uint8_t *lhs, uint8_t *rhs)
{
    uint8_t tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

void
reverse_byte_slice(uint8_t *ptr, size_t len)
{
    for (size_t i = 0; 2 * i < len; ++i) {
        swap_bytes(ptr + i, ptr + len - i - 1);
    }
}

size_t
decreasing_search_from_back(uint8_t *ptr, size_t len, uint8_t value)
{
    for (ptrdiff_t i = len - 1; i >= 0; --i) {
        if (ptr[i] >= value) {
            return i;
        }
    }

    return 0;
}

size_t
decreasing_len_from_back(uint8_t *ptr, size_t len)
{
    if (len <= 1) {
        return len;
    }

    size_t result = 1;

    for (ptrdiff_t i = len - 1; i > 0; --i) {
        if (ptr[i - 1] <= ptr[i]) {
            break;
        }

        result += 1;
    }

    return result;
}

void
next_permutation(uint8_t *ptr, size_t len)
{
    size_t const pivot_index = len - 1 - decreasing_len_from_back(ptr, len);
    size_t const next_pivot_index = decreasing_search_from_back(ptr, len, ptr[pivot_index]);

    swap_bytes(ptr + pivot_index, ptr + next_pivot_index);
    reverse_byte_slice(ptr + pivot_index + 1, len - pivot_index - 1);
}

int
main(void)
{
    size_t size;

    if (1 != scanf("%zu", &size)) {
        fprintf(stderr, "failed to read `size` from stdin\n");
        return EXIT_FAILURE;
    }

    if (size <= 0 || 10 <= size) {
        fprintf(stderr, "invalid input parameter, `size` should be strictly between 0 and 10\n");
        return EXIT_FAILURE;
    }

    uint8_t numbers[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    size_t const n_permutations = factorial((uint8_t) size);

    for (size_t i = 0; i < n_permutations; ++i) {
        for (size_t j = 0; j < size; ++j) {
            putchar('0' + numbers[j]);
        }

        putchar('\n');

        next_permutation(numbers, size);
    }
}
