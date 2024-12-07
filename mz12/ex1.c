#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <string.h>
#include <errno.h>

enum
{
    N_REQUIRED_ARGS = 5,
    EXIT_SYSCALL_FAILURE = 42,
    SYSCALL_FAILURE = -1,
    STDIN = 0,
    STDOUT = 1,
    STDERR = 2,
};

typedef int32_t FileDesc;

FileDesc
open_file(char const *file_name, int32_t flags)
{
    auto const file_desc = open(file_name, flags);

    if (SYSCALL_FAILURE == file_desc) {
        fprintf(stderr, "failed to open '%s': %s\n", file_name, strerror(errno));
        _exit(EXIT_SYSCALL_FAILURE);
    }

    return file_desc;
}

FileDesc
create_file(char const *file_name, int32_t flags)
{
    auto const file_desc = open(file_name, O_WRONLY | O_CREAT | flags, 0660);

    if (SYSCALL_FAILURE == file_desc) {
        fprintf(stderr, "failed to create open '%s': %s\n", file_name, strerror(errno));
        _exit(EXIT_SYSCALL_FAILURE);
    }

    return file_desc;
}

void
duplicate_file_desc(FileDesc from, FileDesc to)
{
    if (SYSCALL_FAILURE == dup2(from, to)) {
        fprintf(stderr, "failed to duplicate file descriptor: %s\n", strerror(errno));
        _exit(EXIT_SYSCALL_FAILURE);
    };
}

void
close_file(FileDesc file)
{
    if (SYSCALL_FAILURE == close(file)) {
        fprintf(stderr, "failed to close file: %s\n", strerror(errno));
        _exit(EXIT_SYSCALL_FAILURE);
    }
}

int
main(int argc, char *argv[])
{
    if (N_REQUIRED_ARGS != argc) {
        fprintf(stderr, "%d arguments requred", N_REQUIRED_ARGS);
        exit(EXIT_FAILURE);
    }

    auto const child_pid = fork();

    if (SYSCALL_FAILURE == child_pid) {
        exit(EXIT_FAILURE);
    }

    if (0 != child_pid) {
        auto status = (int32_t){};
        wait(&status);

        printf("%d\n", status);

        exit(EXIT_SUCCESS);
    }

    auto const input_file = open_file(argv[2], O_RDONLY);
    auto const output_file = create_file(argv[3], O_APPEND);
    auto const error_file = create_file(argv[4], O_TRUNC);

    duplicate_file_desc(input_file, STDIN);
    duplicate_file_desc(output_file, STDOUT);
    duplicate_file_desc(error_file, STDERR);

    close_file(input_file);
    close_file(output_file);
    close_file(error_file);

    if (SYSCALL_FAILURE == execl(argv[1], argv[1], nullptr)) {
        fprintf(stderr, "failed to execute target binary: %s\n", strerror(errno));
        _exit(EXIT_SYSCALL_FAILURE);
    }
}
