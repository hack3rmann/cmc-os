#include <stdio.h>

int
main()
{
    int a, b, c;
    scanf("%d%d", &a, &b);

    c = a + b;

    for (int i = 0; i < 350000000; ++i) {
    }

    printf("%d\n", c);
}
