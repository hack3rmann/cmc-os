#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

[[noreturn]]
void
exit_last(pid_t initial_pid)
{
    if (getpid() == initial_pid) {
        printf("-1\n");
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }

    _exit(EXIT_FAILURE);
}

[[noreturn]]
void
exit_process(pid_t pid, int32_t status)
{
    if (getpid() == pid) {
        exit(status);
    } else {
        _exit(status);
    }
}

void
make_process_recursive(pid_t initial_pid)
{
    auto cur_number = (int32_t){};

    if (EOF == scanf("%d", &cur_number)) {
        exit_process(initial_pid, EXIT_SUCCESS);
    };

    auto const pid = fork();

    if (pid < 0) {
        exit_last(initial_pid);
    }

    if (0 == pid) {
        make_process_recursive(initial_pid);
        return;
    }

    auto status = (int32_t){};
    wait(&status);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        exit_last(initial_pid);
    }

    printf("%d\n", cur_number);
    fflush(stdout);
}

int
main()
{
    auto const self_pid = getpid();
    make_process_recursive(self_pid);
}
