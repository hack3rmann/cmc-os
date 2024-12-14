#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <errno.h>

enum
{
    STR_AMOUNT = 3,
    MAX_LEN = 8,
    BASE_TEN_RADIX = 10,
};

int32_t
int32_parse_prefix(char const *src)
{
    errno = 0;
    auto parse_end = (char *) nullptr;
    auto const result = strtol(src, &parse_end, BASE_TEN_RADIX);

    if (0 != errno || parse_end == src || (int32_t) result != result) {
        fprintf(stderr, "invalid 32-bit number '%s'\n", src);
        _exit(EXIT_FAILURE);
    }

    return result;
}

pid_t
spawn_process(size_t process_index)
{
    auto const pid = fork();

    if (0 != pid) {
        return pid;
    }

    char buf[MAX_LEN + 1] = {};

    if (nullptr == fgets(buf, sizeof(buf), stdin)) {
        _exit(EXIT_FAILURE);
    }

    auto const number = int32_parse_prefix(buf);

    printf("%zu %d\n", process_index + 1, number * number);
    fflush(stdout);

    _exit(EXIT_SUCCESS);
}

int
main(void)
{
    setbuf(stdin, 0);

    for (size_t i = 0; i <= STR_AMOUNT; ++i) {
        [[maybe_unused]]
        auto const pid = spawn_process(i);
    }

    auto status = (int32_t){};
    while (wait(&status) > 0) {
        // empty
    }
}
