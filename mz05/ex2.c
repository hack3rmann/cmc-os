#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static char const PERMISSIONS_TEMPLATE[] = "rwxrwxrwx";

enum
{
    PERMISSIONS_ARG_START_INDEX = 1,
    BASE_EIGHT_RADIX = 8,
    MIN_PERMISSIONS_VALUE = 0,
    PERMISSIONS_STRING_LEN = sizeof(PERMISSIONS_TEMPLATE) - 1,
    N_PERMISSIONS_BITS = PERMISSIONS_STRING_LEN,
    MAX_PERMISSIONS_VALUE = (1 << N_PERMISSIONS_BITS) - 1,
};

typedef struct PermissionsDisplayed
{
    char string[PERMISSIONS_STRING_LEN + 1];
} PermissionsDisplayed;

typedef uint16_t Permissions;

Permissions
Permissions_parse(char const *src)
{
    errno = 0;
    char *parse_end = nullptr;
    auto const parsed = strtol(src, &parse_end, BASE_EIGHT_RADIX);

    if (0 != errno || '\0' != *parse_end || parse_end == src || parsed < MIN_PERMISSIONS_VALUE ||
        parsed > MAX_PERMISSIONS_VALUE) {

        fprintf(stderr, "invalid %d-bit value '%s'\n", N_PERMISSIONS_BITS, src);
        exit(EXIT_FAILURE);
    }

    return (Permissions) parsed;
}

PermissionsDisplayed
Permissions_display(Permissions self)
{
    auto result = (PermissionsDisplayed){};
    strncpy(result.string, PERMISSIONS_TEMPLATE, sizeof(result.string));

    uint32_t cur_bits = self;

    for (size_t i = 0; i < PERMISSIONS_STRING_LEN; ++i, cur_bits >>= 1) {
        if (1 & ~cur_bits) {
            result.string[PERMISSIONS_STRING_LEN - i - 1] = '-';
        }
    }

    return result;
}

int
main(int argc, char *argv[])
{
    for (size_t i = PERMISSIONS_ARG_START_INDEX; i < (size_t) argc; ++i) {
        auto const permissions = Permissions_parse(argv[i]);
        auto const permissions_displayed = Permissions_display(permissions);

        printf("%s\n", permissions_displayed.string);
    }
}
