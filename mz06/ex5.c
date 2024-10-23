#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void
print_allocation_error_message()
{
    fprintf(stderr, "failed to allocate memory\n");
}

typedef struct VecStr
{
    char **ptr;
    size_t len;
    size_t cap;
} VecStr;

#define VecStr_DEFAULT ((VecStr){.ptr = nullptr, .len = 0, .cap = 0})
#define VecStr_INITIAL_CAPACITY 2
#define VecStr_GROWTH_RATE 2

void
VecStr_push(VecStr *self, char const *str)
{
    if (nullptr == self->ptr) {
        self->cap = VecStr_INITIAL_CAPACITY;
        self->ptr = calloc(self->cap, sizeof(*self->ptr));

        if (nullptr == self->ptr) {
            print_allocation_error_message();
            exit(EXIT_FAILURE);
        }
    } else if (self->len == self->cap) {
        self->cap *= VecStr_GROWTH_RATE;
        self->ptr = realloc(self->ptr, sizeof(*self->ptr) * self->cap);

        if (nullptr == self->ptr) {
            print_allocation_error_message();
            exit(EXIT_FAILURE);
        }
    }

    char *str_clone = nullptr;

    if (asprintf(&str_clone, "%s", str) < 0) {
        print_allocation_error_message();
        exit(EXIT_FAILURE);
    }

    self->ptr[self->len] = str_clone;
    self->len += 1;
}

void
VecStr_drop(VecStr *self)
{
    for (size_t i = 0; i < self->len; ++i) {
        free(self->ptr[i]);
    }

    free(self->ptr);

    *self = VecStr_DEFAULT;
}

typedef struct StrBuf
{
    char *ptr;
    /// length without '\0'
    size_t len;
    /// capacity + '\0'
    size_t cap;
} StrBuf;

StrBuf
StrBuf_from_ptr(char *ptr, size_t size)
{
    return (StrBuf){.ptr = ptr, .len = 0, .cap = size};
}

void
StrBuf_append_lossy(StrBuf *self, char const *source_str)
{
    if (self->len + 1 >= self->cap) {
        return;
    }

    strncpy(self->ptr + self->len, source_str, self->cap - self->len - 1);
    self->len += strlen(source_str);
}

enum
{
    GETCWD2_FAILURE = -1,
    INVALAID_FILE_DESC = -1,
    SYSCALL_FAILURE = -1,
};

ssize_t
getcwd2(int dir_file_desc, char *result_buf, size_t result_buf_size)
{
    auto const process_dir = opendir(".");

    if (nullptr == process_dir) {
        return GETCWD2_FAILURE;
    }

    auto const process_dir_file_desc = dirfd(process_dir);

    if (INVALAID_FILE_DESC == process_dir_file_desc) {
        // Cleanup
        closedir(process_dir);
        return GETCWD2_FAILURE;
    }

    if (SYSCALL_FAILURE == fchdir(dir_file_desc)) {
        // Cleanup
        closedir(process_dir);
        return GETCWD2_FAILURE;
    }

    auto root_stat = (struct stat){};

    if (SYSCALL_FAILURE == lstat("/", &root_stat)) {
        // Cleanup
        fchdir(process_dir_file_desc);
        closedir(process_dir);

        return GETCWD2_FAILURE;
    }

    auto cur_file_stat = (struct stat){};

    if (SYSCALL_FAILURE == fstat(dir_file_desc, &cur_file_stat)) {
        // Cleanup
        fchdir(process_dir_file_desc);
        closedir(process_dir);

        return GETCWD2_FAILURE;
    }

    auto prev_file_stat = cur_file_stat;

    // CWD is `/`
    if (prev_file_stat.st_ino == root_stat.st_ino && prev_file_stat.st_dev == root_stat.st_dev) {
        strncpy(result_buf, "/", result_buf_size);

        if (SYSCALL_FAILURE == fchdir(process_dir_file_desc)) {
            // Cleanup
            closedir(process_dir);
            return GETCWD2_FAILURE;
        }

        if (SYSCALL_FAILURE == closedir(process_dir)) {
            return GETCWD2_FAILURE;
        }

        return sizeof("/") - 1;
    }

    auto paths_backwards = VecStr_DEFAULT;

    while (prev_file_stat.st_ino != root_stat.st_ino || prev_file_stat.st_dev != root_stat.st_dev) {
        if (SYSCALL_FAILURE == chdir("..")) {
            // Cleanup
            VecStr_drop(&paths_backwards);
            fchdir(process_dir_file_desc);
            closedir(process_dir);

            return GETCWD2_FAILURE;
        }

        auto const cur_dir = opendir(".");

        if (nullptr == cur_dir) {
            // Cleanup
            VecStr_drop(&paths_backwards);
            fchdir(process_dir_file_desc);
            closedir(process_dir);

            return GETCWD2_FAILURE;
        }

        for (auto entry = readdir(cur_dir); nullptr != entry; entry = readdir(cur_dir)) {
            char *path_from_cur = nullptr;

            if (asprintf(&path_from_cur, "./%s", entry->d_name) < 0) {
                print_allocation_error_message();

                // Cleanup
                closedir(cur_dir);
                VecStr_drop(&paths_backwards);
                fchdir(process_dir_file_desc);
                closedir(process_dir);

                // Allocation error is fatal failure
                exit(EXIT_FAILURE);
            }

            if (SYSCALL_FAILURE == lstat(path_from_cur, &cur_file_stat)) {
                // Cleanup
                free(path_from_cur);
                closedir(cur_dir);
                VecStr_drop(&paths_backwards);
                fchdir(process_dir_file_desc);
                closedir(process_dir);

                return GETCWD2_FAILURE;
            }

            free(path_from_cur);

            if (cur_file_stat.st_ino == prev_file_stat.st_ino && cur_file_stat.st_dev == prev_file_stat.st_dev) {
                VecStr_push(&paths_backwards, entry->d_name);
                break;
            }
        }

        if (SYSCALL_FAILURE == lstat(".", &prev_file_stat)) {
            // Cleanup
            closedir(cur_dir);
            VecStr_drop(&paths_backwards);
            fchdir(process_dir_file_desc);
            closedir(process_dir);

            return GETCWD2_FAILURE;
        }

        if (SYSCALL_FAILURE == closedir(cur_dir)) {
            // Cleanup
            VecStr_drop(&paths_backwards);
            fchdir(process_dir_file_desc);
            closedir(process_dir);

            return GETCWD2_FAILURE;
        }
    }

    auto buf = StrBuf_from_ptr(result_buf, result_buf_size);
    size_t result_size = 0;

    for (auto i = (ssize_t) paths_backwards.len - 1; i >= 0; --i) {
        StrBuf_append_lossy(&buf, "/");
        StrBuf_append_lossy(&buf, paths_backwards.ptr[i]);

        result_size += 1 + strlen(paths_backwards.ptr[i]);
    }

    VecStr_drop(&paths_backwards);

    if (SYSCALL_FAILURE == fchdir(process_dir_file_desc)) {
        closedir(process_dir);
        return GETCWD2_FAILURE;
    }

    if (SYSCALL_FAILURE == closedir(process_dir)) {
        return GETCWD2_FAILURE;
    }

    return result_size;
}
