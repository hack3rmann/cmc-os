#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum
{
    MAX_LINE_LEN = 128
};

int
main()
{
    char s1[MAX_LINE_LEN + 1], s2[MAX_LINE_LEN + 1], s3[MAX_LINE_LEN + 1];

    if (NULL == fgets(s1, MAX_LINE_LEN + 1, stdin) || NULL == fgets(s2, MAX_LINE_LEN + 1, stdin) ||
        NULL == fgets(s3, MAX_LINE_LEN + 1, stdin)) {

        fprintf(stderr, "invalid input string\n");
        exit(EXIT_FAILURE);
    }

    size_t s1_len = strlen(s1);
    size_t s2_len = strlen(s2);
    size_t s3_len = strlen(s3);

    s1[--s1_len] = '\0';
    s2[--s2_len] = '\0';
    s3[--s3_len] = '\0';

    if (0 > printf("[Host:%s,Login:%s,Password:%s]\n", s1, s2, s3)) {
        fprintf(stderr, "failed to write to stdout\n");
        exit(EXIT_FAILURE);
    }

    if (0 != fflush(stdout)) {
        fprintf(stderr, "failed to flush stdout\n");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
