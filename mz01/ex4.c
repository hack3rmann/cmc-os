#include <stdlib.h>

int
int_even_odd_comparator(void const *lhs, void const *rhs)
{
    int const l = *(int *) lhs;
    int const r = *(int *) rhs;

    int const cmp_type = ((l & 1) << 1) | (r & 1);

    switch (cmp_type) {
    case 0b00:
        return (l > r) - (l < r);
    case 0b01:
        return -1;
    case 0b10:
        return 1;
    case 0b11:
        return (l < r) - (l > r);
    default:
        return 0;
    }
}

void
sort_even_odd(size_t count, int *data)
{
    qsort(data, count, sizeof(*data), int_even_odd_comparator);
}
