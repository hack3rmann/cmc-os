#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char *argv[], char *envp[])
{
    for (size_t i = 0; envp[i] != 0; ++i) {
        printf("envp[%zu] = `%s`\n", i, envp[i]);
    }
}
