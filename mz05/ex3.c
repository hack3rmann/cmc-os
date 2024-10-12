#include <stdint.h>
#include <stdlib.h>

enum
{
    PARSE_RWX_PERMISSIONS_FAILURE = -1,
    DISALLOWED_PERMISSION = '-',
};

int
parse_rwx_permissions(char const *src)
{
    if (nullptr == src) {
        return PARSE_RWX_PERMISSIONS_FAILURE;
    }

    static auto const REFERENCE = "rwxrwxrwx";
    uint32_t result = 0;
    size_t i = 0;

    for (; '\0' != src[i] && '\0' != REFERENCE[i]; ++i) {
        result <<= 1;

        if (DISALLOWED_PERMISSION == src[i]) {
            continue;
        } else if (REFERENCE[i] == src[i]) {
            result |= 1;
        } else {
            return PARSE_RWX_PERMISSIONS_FAILURE;
        }
    }

    if ('\0' != src[i] || '\0' != REFERENCE[i]) {
        return PARSE_RWX_PERMISSIONS_FAILURE;
    }

    return (int) result;
}
