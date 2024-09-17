// typedef int STYPE;
// typedef unsigned UTYPE;

STYPE
bit_reverse(STYPE value)
{
    UTYPE bits = ~(UTYPE) 0;
    UTYPE value_unsigned = (UTYPE) value;
    UTYPE result_unsigned = 0;

    while (0 != bits) {
        result_unsigned <<= 1;
        result_unsigned |= value_unsigned & 1;
        value_unsigned >>= 1;
        bits >>= 1;
    }

    return (STYPE) result_unsigned;
}
