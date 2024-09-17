#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>

typedef struct Elem
{
    struct Elem *next;
    char *str;
} Elem;

char *
increment_32bit(char *value)
{
    if (nullptr == value) {
        return nullptr;
    }

    char *end = nullptr;
    errno = 0;

    long const integer = strtol(value, &end, 10);

    if ('\0' != *end || 0 != errno || value == end || integer >= (long) INT_MAX || integer < (long) INT_MIN - 1) {
        return nullptr;
    }

    char *result = nullptr;

    asprintf(&result, "%d", (int) integer + 1);

    return result;
}

Elem *
dup_elem(Elem *head)
{
    if (nullptr == head) {
        return nullptr;
    }

    for (Elem *prev = nullptr, *cur = head; nullptr != cur; prev = cur, cur = cur->next) {
        auto const cur_string = increment_32bit(cur->str);

        if (nullptr == cur_string) {
            continue;
        }

        Elem *const new_elem = malloc(sizeof(*new_elem));
        *new_elem = (Elem){
            .str = cur_string,
            .next = cur,
        };

        if (nullptr == prev) {
            head = new_elem;
        } else {
            prev->next = new_elem;
        }

        prev = new_elem;
    }

    return head;
}

int
main()
{
    Elem list[] = {
        (Elem){.next = list + 1, .str = "10"},
        (Elem){.next = list + 2, .str = "5x"},
        (Elem){.next = list + 3, .str = "alpha"},
        (Elem){.next = nullptr, .str = "-03"},
    };

    auto const new_list = dup_elem(list);

    for (auto cur = new_list; nullptr != cur; cur = cur->next) {
        printf("'%s', ", cur->str);
    }
}
