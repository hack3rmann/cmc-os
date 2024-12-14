#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

enum
{
    MODE_PARALLEL = 'p',
    MODE_SEQUENTIAL = 's',
};

pid_t
spawn_path_program(char const *name)
{
    auto const pid = fork();

    if (pid < 0) {
        fprintf(stderr, "fork failed: '%s'\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (0 != pid) {
        return pid;
    }

    if (-1 == execlp(name, name, nullptr)) {
        fprintf(stderr, "failed to execute '%s': '%s'\n", name, strerror(errno));
        _exit(EXIT_FAILURE);
    }

    return 0;
}

size_t
wait_all()
{
    auto status = (int32_t){};
    auto success_count = (size_t) 0;
    errno = 0;

    while (true) {
        auto const pid = wait(&status);

        if (pid < 0) {
            if (ECHILD == errno) {
                break;
            } else if (EINTR == errno) {
                continue;
            } else {
                fprintf(stderr, "wait failed: '%s'\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

        if (WIFEXITED(status) && EXIT_SUCCESS == WEXITSTATUS(status)) {
            success_count += 1;
        }
    }

    return success_count;
}

int
main(int argc, char *argv[])
{
    auto success_count = (size_t) 0;

    for (size_t i = 1; i < (size_t) argc; ++i) {
        auto const name = argv[i] + 1;
        auto const mode = argv[i][0];

        switch (mode) {
        case MODE_SEQUENTIAL: {
            success_count += wait_all();
            spawn_path_program(name);
            success_count += wait_all();
        } break;
        case MODE_PARALLEL: {
            spawn_path_program(name);
        } break;
        default: {
            fprintf(stderr, "invalid execution mode '%c'\n", mode);
            exit(EXIT_FAILURE);
        }
        }
    }

    success_count += wait_all();

    printf("%zu\n", success_count);
}
