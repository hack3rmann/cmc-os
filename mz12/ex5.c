#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>

enum
{
    RANDOM_BYTES_CNT = 16,
    UINT8_T_IN_HEX_LEN = 2,
};

size_t constexpr MAX_N_TRIES = 10000;
size_t constexpr N_REQUIRED_ARGS = 2;

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

void
make_unique_file_name(char const *parent_dir, char const *name_prefix, char const *name_suffix,
                      char result_path[PATH_MAX])
{
    for (size_t file_index = 0; file_index < MAX_N_TRIES; ++file_index) {
        if (snprintf(result_path, PATH_MAX, "%s/%s%zu%s", parent_dir, name_prefix, file_index, name_suffix) >
            PATH_MAX) {
            fprintf(stderr, "failed to create temporary file\n");
            exit(EXIT_FAILURE);
        }

        errno = 0;
        auto file_stat = (struct stat){};

        if (-1 == lstat(result_path, &file_stat) && ENOENT == errno) {
            break;
        }
    }
}

FILE *
create_temporary_file(char const *parent_dir, char const *name_prefix, char const *name_suffix,
                      char result_path[PATH_MAX])
{
    make_unique_file_name(parent_dir, name_prefix, name_suffix, result_path);

    auto const result = fopen(result_path, "w");

    if (nullptr == result) {
        fprintf(stderr, "failed to create temporary file\n");
        exit(EXIT_FAILURE);
    }

    return result;
}

int (*evaluate_expression_at)(int) = nullptr;

int
main(int argc, const char *argv[])
{
    if ((size_t) argc != N_REQUIRED_ARGS) {
        fprintf(stderr, "%zu arguments are required\n", N_REQUIRED_ARGS);
        exit(EXIT_FAILURE);
    }

    char source_file_path[PATH_MAX] = {};

    auto const temp_dir_name = get_temporary_directory_name();
    auto const source_file = create_temporary_file(temp_dir_name, "src", ".c", source_file_path);

    auto const expression_source = argv[1];
    auto const source_write_result = fprintf(source_file,
                                             "#include <stdio.h>\n"
                                             "int constexpr reject = 0;\n"
                                             "int constexpr summon = 1;\n"
                                             "int constexpr disqualify = 2;\n"
                                             "int evaluate_expression_at(int x) { return (%s); }\n",
                                             expression_source);

    if (source_write_result < 0) {
        fprintf(stderr, "failed to write source file '%s': %s\n", source_file_path, strerror(errno));
        unlink(source_file_path);
        exit(EXIT_FAILURE);
    }

    if (0 != fclose(source_file)) {
        fprintf(stderr, "failed to close file '%s': %s\n", source_file_path, strerror(errno));
        unlink(source_file_path);
        exit(EXIT_FAILURE);
    }

    char lib_file_path[PATH_MAX];
    make_unique_file_name(temp_dir_name, "lib", ".so", lib_file_path);

    auto const gcc_pid = fork();

    if (gcc_pid < 0) {
        fprintf(stderr, "failed to fork: %s\n", strerror(errno));
        unlink(source_file_path);
        exit(EXIT_FAILURE);
    } else if (0 == gcc_pid && -1 == execlp("gcc", "gcc", "-std=gnu23", "-O3", "-shared", "-fPIC", source_file_path,
                                            "-o", lib_file_path, nullptr)) {
        fprintf(stderr, "failed to execute 'gcc -shared -fPIC %s -o %s'\n", source_file_path, lib_file_path);
        _exit(EXIT_FAILURE);
    } else {
        auto status = (int32_t){};

        if (-1 == waitpid(gcc_pid, &status, 0)) {
            fprintf(stderr, "failed to wait for gcc with pid=%d\n", gcc_pid);
            unlink(source_file_path);
            unlink(lib_file_path);
            exit(EXIT_FAILURE);
        }

        if (!WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
            fprintf(stderr, "failed to compile the expression '%s'\n", expression_source);
            unlink(source_file_path);
            unlink(lib_file_path);
            exit(EXIT_FAILURE);
        }
    }

    unlink(source_file_path);

    auto const handle = dlopen(lib_file_path, RTLD_NOW);

    if (nullptr == handle) {
        fprintf(stderr, "failed to load shared library: %s\n", dlerror());
        unlink(lib_file_path);
        exit(EXIT_FAILURE);
    }

    evaluate_expression_at = dlsym(handle, "evaluate_expression_at");

    if (nullptr == evaluate_expression_at) {
        fprintf(stderr, "failed to find symbol 'int evaluate_expression_at(int): %s\n", dlerror());
        dlclose(handle);
        unlink(lib_file_path);
        exit(EXIT_FAILURE);
    }

    unlink(lib_file_path);

    auto value = (int32_t){};

    while (1 == scanf("%d", &value)) {
        char const *const result_strings[] = {"reject", "summon", "disqualify"};
        auto constexpr N_RESULT_STRINGS = sizeof(result_strings) / sizeof(*result_strings);

        auto const exit_result = evaluate_expression_at(value);

        if (exit_result < 0 || (size_t) exit_result >= N_RESULT_STRINGS) {
            fprintf(stderr, "invalid result for 'evaluate_expression_at' call: %d\n", exit_result);
            exit(EXIT_FAILURE);
        }

        auto const result_string = result_strings[(size_t) exit_result];

        printf("%s\n", result_string);
    }

    dlclose(handle);
}
