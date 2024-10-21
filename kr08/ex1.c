#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

char
char_shift(char value, uint8_t shift)
{
    if ('a' <= value && value <= 'z') {
        value = 'a' + ((value - 'a') + shift) % ('z' - 'a' + 1);
    }

    return value;
}

uint8_t
get_shift(size_t index)
{
    index = index % 15;

    if (index <= 13) {
        return 13 - index;
    } else if (14 == index) {
        return 25;
    } else {
        exit(1);
    }
}

void
replace_crl_to_lf_at_end(char *str)
{
    auto const str_len = strlen(str);

    if (0 == str_len) {
        return;
    } else if (1 == str_len && str[0] == '\r') {
        str[0] = '\n';
    } else if (str_len >= 2) {
        if ('\r' == str[str_len - 2] && '\n' == str[str_len - 1]) {
            str[str_len - 2] = '\n';
            str[str_len - 1] = '\0';
        } else if ('\r' == str[str_len - 1]) {
            str[str_len - 1] = '\n';
        }
    }
}

int
main(void)
{
    char cur_string[101] = {};
    size_t string_index = 0;

    while (nullptr != fgets(cur_string, sizeof(cur_string), stdin)) {
        for (size_t i = 0; '\0' != cur_string[i]; ++i) {
            cur_string[i] = char_shift(cur_string[i], get_shift(string_index));
        }

        replace_crl_to_lf_at_end(cur_string);
        printf("%s", cur_string);

        string_index += 1;
    }
}
