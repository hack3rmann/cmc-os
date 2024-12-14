#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int
main()
{
    auto n_processes = (uint32_t){};

    if (1 != scanf("%u", &n_processes)) {
        exit(EXIT_FAILURE);
    }

    if (0 == n_processes) {
        fprintf(stderr, "process count should be > 0\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < n_processes; i++) {
        if (i == n_processes - 1) {
            printf("%u\n", i + 1);
        } else {
            printf("%u ", i + 1);
        }

        fflush(stdout);

        if (0 != fork()) {
            wait(nullptr);
            exit(EXIT_SUCCESS);
        }
    }
}
