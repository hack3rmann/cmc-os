int
satsum(int lhs, int rhs)
{
    enum
    {
        NEG_ONE = (int) ~0u,
        ONE = -NEG_ONE,
        MY_INT_MAX = (int) ((unsigned) NEG_ONE >> ONE),
        MY_INT_MIN = -MY_INT_MAX - ONE,
    };

    int result;

    if (__builtin_add_overflow(lhs, rhs, &result)) {
        if (result < 0) {
            return MY_INT_MAX;
        } else {
            return MY_INT_MIN;
        }
    }

    return result;
}
