#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

enum
{
    ONE_KIBIBYTE = 1 << 10,
    ONE_KIBIBYTE_MASK = ONE_KIBIBYTE - 1,
    SYSCALL_FAILURE = -1,
    PATHS_ARGUMENTS_START = 1,
    N_HARD_LINKS = 1,
};

int
main(int argc, char *argv[])
{
    size_t size = 0;

    for (size_t i = PATHS_ARGUMENTS_START; i < (size_t) argc; ++i) {
        auto const file_path = argv[i];
        auto file_stat = (struct stat){};

        if (SYSCALL_FAILURE != lstat(file_path, &file_stat) && 0 == (file_stat.st_size & ONE_KIBIBYTE_MASK) &&
            N_HARD_LINKS == file_stat.st_nlink && S_ISREG(file_stat.st_mode)) {
            size += file_stat.st_size;
        }
    }

    printf("%zu\n", size);
}
