#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

enum
{
    N_NUMBERS = '9' - '0' + 1,
    N_LETTERS = 'z' - 'a' + 1,
    NUMERIC_VALUES_START = 1,
    LOWERCASE_LETTERS_START = NUMERIC_VALUES_START + N_NUMBERS,
    UPPERCASE_LETTERS_START = LOWERCASE_LETTERS_START + N_LETTERS,
    LAST_CODE = UPPERCASE_LETTERS_START + N_LETTERS
};

uint8_t
decode_symbol(char value)
{
    if ('@' == value) {
        return 0;
    } else if ('0' <= value && value <= '9') {
        return value - '0' + NUMERIC_VALUES_START;
    } else if ('a' <= value && value <= 'z') {
        return value - 'a' + LOWERCASE_LETTERS_START;
    } else if ('A' <= value && value <= 'Z') {
        return value - 'A' + UPPERCASE_LETTERS_START;
    } else if ('#' == value) {
        return LAST_CODE;
    }

    return 0;
}

char
encode_symbol(uint8_t value)
{
    static char const TABLE[] = "@0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ#";
    size_t const table_size = sizeof(TABLE) - 1;

    if (value >= table_size) {
        return TABLE[table_size - 1];
    }

    return TABLE[(size_t) value];
}

enum
{
    BRAKE_SYMBOL_SECOND_MASK = 1 << 2,
    BRAKE_SYMBOL_THIRD_MASK = 1 << 3
};

char
brake_symbol(char value)
{
    uint8_t const symbol = decode_symbol(value);
    uint8_t const read_symbol = (symbol ^ BRAKE_SYMBOL_THIRD_MASK) & ~BRAKE_SYMBOL_SECOND_MASK;

    return encode_symbol(read_symbol);
}

bool
char_is_alphanumeric(char value)
{
    return ('0' <= value && value <= '9') || ('a' <= value && value <= 'z') || ('A' <= value && value <= 'Z');
}

int
main()
{
    for (char cur_symbol = getchar(); !feof(stdin); cur_symbol = getchar()) {
        if (!char_is_alphanumeric(cur_symbol)) {
            continue;
        }

        putchar(brake_symbol(cur_symbol));
    }

    putchar('\n');

    return EXIT_SUCCESS;
}
