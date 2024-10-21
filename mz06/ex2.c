#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct Str
{
    char *ptr;
    size_t len;
} Str;

#define Str_DEFAULT ((Str){.ptr = nullptr, .len = 0})
#define Str(c_string_literal) ((Str){.ptr = c_string_literal, .len = sizeof(c_string_literal) - 1})

Str
Str_from_c(char *cstr)
{
    return (Str){.ptr = cstr, .len = strlen(cstr)};
}

bool
Str_eq(Str self, Str other)
{
    if (self.len != other.len) {
        return false;
    }

    for (size_t i = 0; i < self.len; ++i) {
        if (self.ptr[i] != other.ptr[i]) {
            return false;
        }
    }

    return true;
}

Str
Str_split_once(Str *self, char delim)
{
    size_t delim_index = SIZE_MAX;

    for (size_t i = 0; i < self->len; ++i) {
        if (self->ptr[i] == delim) {
            delim_index = i;
            break;
        }
    }

    if (SIZE_MAX == delim_index) {
        return Str_DEFAULT;
    }

    auto result = (Str){.ptr = self->ptr, .len = delim_index};

    self->ptr += delim_index + 1;
    self->len -= delim_index + 1;

    if (0 == self->len) {
        *self = Str_DEFAULT;
    }

    return result;
}

char *
return_write_back(char *write_ptr)
{
    while ('/' != *write_ptr) {
        write_ptr -= 1;
    }

    return write_ptr;
}

void
normalize_path(char *path)
{
    auto write_ptr = path;
    auto look_ahead = Str_from_c(path);

    [[maybe_unused]]
    auto const pre_root = Str_split_once(&look_ahead, '/');

    // path is "/"
    if (0 == look_ahead.len) {
        return;
    }

    while (0 != look_ahead.len) {
        auto segment = Str_split_once(&look_ahead, '/');

        if (0 == segment.len && 0 != look_ahead.len) {
            segment = look_ahead;
            look_ahead = Str_DEFAULT;
        }

        if (Str_eq(segment, Str("."))) {
            continue;
        } else if (Str_eq(segment, Str(".."))) {
            write_ptr -= 1;
            write_ptr = return_write_back(write_ptr);
        } else {
            *write_ptr = '/';
            write_ptr += 1;

            memmove(write_ptr, segment.ptr, segment.len);
            write_ptr += segment.len;
        }
    }

    if (path == write_ptr) {
        path[0] = '/';
        path[1] = '\0';
    } else {
        *write_ptr = '\0';
    }
}

// int
// main(int argc, char *argv[])
// {
//     if (argc < 2) {
//         return 1;
//     }

//     auto path = argv[1];
//     normalize_path(path);

//     printf("%s\n", path);
// }
