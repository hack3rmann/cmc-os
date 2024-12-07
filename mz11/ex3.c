#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <stdio.h>
#include <errno.h>

enum
{
    MAX_STRING_LEN = 8,
    BASE_TEN_RADIX = 10
};

int32_t
int32_parse(char const *src)
{
    errno = 0;
    auto parse_end = (char *) nullptr;
    auto const result = strtol(src, &parse_end, BASE_TEN_RADIX);

    if (0 == errno || '\0' != *parse_end || parse_end == src || (int32_t) result != result) {
        fprintf(stderr, "invalid 32-bit integer '%s'", src);
    }

    return (int32_t) result;
}

void
spawn_son(int32_t forward_pipe[2], int32_t backward_pipe[2], int32_t son_number)
{
    auto const pid = fork();

    if (0 != pid) {
        return;
    }

    if (nullptr != forward_pipe) {
        int32_t ready_to_scan_flag;
        read(forward_pipe[0], &ready_to_scan_flag, sizeof(ready_to_scan_flag));

        fflush(stdout);
    }

    char number_src[MAX_STRING_LEN];
    fgets(number_src, sizeof(number_src), stdin);
    getchar();

    if (nullptr != backward_pipe) {
        int32_t finished_reading = 1;
        write(backward_pipe[1], &finished_reading, sizeof(finished_reading));

        fflush(stdout);
    }

    auto const number = int32_parse(number_src);

    printf("%d %d\n", son_number, number * number);
    fflush(stdout);

    _exit(0);
}

int
main()
{
    setbuf(stdin, 0);
    int first_son_pipe_backward[2];
    int second_son_pipe_forward[2];
    int second_son_pipe_backward[2];
    int third_son_pipe_forward[2];

    pipe(first_son_pipe_backward);
    pipe(second_son_pipe_forward);
    pipe(second_son_pipe_backward);
    pipe(third_son_pipe_forward);

    spawn_son(nullptr, first_son_pipe_backward, 1);
    spawn_son(second_son_pipe_forward, second_son_pipe_backward, 2);
    spawn_son(third_son_pipe_forward, nullptr, 3);

    int32_t son_finished_reading;

    read(first_son_pipe_backward[0], &son_finished_reading, sizeof(son_finished_reading));

    write(second_son_pipe_forward[1], &son_finished_reading, sizeof(son_finished_reading));
    read(second_son_pipe_backward[0], &son_finished_reading, sizeof(son_finished_reading));

    write(third_son_pipe_forward[1], &son_finished_reading, sizeof(son_finished_reading));

    int32_t status;
    while (wait(&status) != -1) {
    }
}
