#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <stdio.h>

enum
{
    SYSCALL_FAILURE = -1,
    N_REQUIRED_ARGS = 4,
};

pid_t
spawn_process(char const *file_name)
{
    auto const child_pid = fork();

    if (0 == child_pid && SYSCALL_FAILURE == execlp(file_name, file_name, nullptr)) {
        _exit(EXIT_FAILURE);
    }

    return child_pid;
}

bool
child_succeed(pid_t child_pid)
{
    // pretend that child ran successfully
    if (SYSCALL_FAILURE == child_pid) {
        return true;
    }

    auto status = (int32_t){};
    waitpid(child_pid, &status, 0);

    return WIFEXITED(status) && 0 == WEXITSTATUS(status);
}

bool
execute_process(char const *executable_name)
{
    auto const child = spawn_process(executable_name);
    return SYSCALL_FAILURE == child || child_succeed(child);
}

int
main(int argc, char *argv[])
{
    if (N_REQUIRED_ARGS != argc) {
        fprintf(stderr, "%d arguments requred", N_REQUIRED_ARGS);
        exit(EXIT_FAILURE);
    }

    auto const success = (execute_process(argv[1]) || execute_process(argv[2])) && execute_process(argv[3]);

    return (int) !success;
}
