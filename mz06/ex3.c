#include <stdlib.h>

struct s1
{
    char f1;
    long long f2;
    char f3;
};

struct s2
{
    long long f2;
    char f1;
    char f3;
};

size_t
compactify(void *ptr, size_t size)
{
    if (0 == size) {
        return 0;
    }

    auto const before = (struct s1 *) ptr;
    auto const after = (struct s2 *) ptr;

    for (size_t i = 0; sizeof(*before) * i < size; ++i) {
        auto const cur = before[i];

        after[i] = (struct s2){.f1 = cur.f1, .f2 = cur.f2, .f3 = cur.f3};
    }

    auto const result_size = size / sizeof(*before) * sizeof(*after);

    return result_size;
}
