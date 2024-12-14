#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

size_t constexpr MIN_N_ARGS = 2;
size_t constexpr MAX_N_TRIES = 10000;

char const *
get_temporary_directory_name()
{
    auto result = getenv("XDG_RUNTIME_DIR");

    if (nullptr != result) {
        return result;
    }

    result = getenv("TMPDIR");

    if (nullptr != result) {
        return result;
    }

    return "/tmp";
}

FILE *
create_temporary_file(char const *parent_dir, char result_path[PATH_MAX])
{
    for (size_t file_index = 0; file_index < MAX_N_TRIES; ++file_index) {
        if (snprintf(result_path, PATH_MAX, "%s/script%zu.py", parent_dir, file_index) > PATH_MAX) {
            fprintf(stderr, "failed to create temporary file\n");
            exit(EXIT_FAILURE);
        }

        errno = 0;
        auto file_stat = (struct stat){};

        if (-1 == lstat(result_path, &file_stat) && ENOENT == errno) {
            break;
        }
    }

    auto const result = fopen(result_path, "w");

    if (nullptr == result) {
        fprintf(stderr, "failed to create temporary file\n");
        exit(EXIT_FAILURE);
    }

    return result;
}

int
main(int argc, const char *argv[])
{
    if ((size_t) argc < MIN_N_ARGS) {
        fprintf(stderr, "minimum of %zu arguments are required\n", MIN_N_ARGS);
        exit(EXIT_FAILURE);
    }

    char script_path[PATH_MAX] = {};

    auto const temp_dir_name = get_temporary_directory_name();
    auto const script_file = create_temporary_file(temp_dir_name, script_path);

    if (-1 == chmod(script_path, S_IRWXU)) {
        fprintf(stderr, "chmod failed: %s\n", strerror(errno));
        fclose(script_file);

        exit(EXIT_FAILURE);
    }

    fprintf(script_file, "#!/usr/bin/python3\nimport os\nprint(");

    for (size_t i = 1; i < (size_t) argc; ++i) {
        fprintf(script_file, "%s", argv[i]);

        if (i + 1 != (size_t) argc) {
            fputc('*', script_file);
        }
    }

    fprintf(script_file, ")\nos.unlink(__file__)\n");

    fclose(script_file);

    if (-1 == execl(script_path, script_path, nullptr)) {
        fprintf(stderr, "execl failed: %s\n", strerror(errno));
        unlink(script_path);
    }

    return EXIT_FAILURE;
}
