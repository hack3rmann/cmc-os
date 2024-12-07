#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int
main(void)
{
    auto const child_pid = fork();

    if (0 != child_pid) {
        int status;
        waitpid(child_pid, &status, 0);
        printf("1\n");
        fflush(stdout);

        return 0;
    }

    auto const grandchild_pid = fork();

    if (0 != grandchild_pid) {
        int status;
        waitpid(grandchild_pid, &status, 0);
        printf("2 ");
        fflush(stdout);

        return 0;
    }

    printf("3 ");
    fflush(stdout);
}
