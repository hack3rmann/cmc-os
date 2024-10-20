#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

void
print_allocation_failure_message()
{
    fprintf(stderr, "failed to allocate memory: %s\n", strerror(errno));
}

typedef struct Str
{
    char const *ptr;
    size_t len;
} Str;

#define Str_DEFAULT ((Str){.ptr = nullptr, .len = 0})
#define Str(c_string_literal) ((Str){.ptr = c_string_literal, .len = sizeof(c_string_literal) - 1})

Str
Str_from_c(char const *cstr)
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

Str
Str_slice(Str self, size_t start, size_t end_exclusive)
{
    if (start > end_exclusive || end_exclusive > self.len) {
        fprintf(stderr, "invalid slice\n");
        exit(EXIT_FAILURE);
    }

    if (start == end_exclusive) {
        return Str_DEFAULT;
    }

    auto result = (Str){
        .ptr = self.ptr + start,
        .len = end_exclusive - start,
    };

    if (0 == result.len) {
        return Str_DEFAULT;
    }

    return result;
}

typedef struct String
{
    char *ptr;
    size_t len;
    size_t cap;
} String;

#define String_DEFAULT ((String){.ptr = nullptr, .len = 0, .cap = 0})
#define String_INITIAL_CAP 16
#define String_GROWTH_NUMERATOR 3
#define String_GROWTH_DENOMINATOR 2
#define String_GROWTH_SHIFT 1

void
String_push(String *self, char value)
{
    if (0 == self->cap) {
        self->cap = String_INITIAL_CAP;
        self->ptr = calloc(self->cap, sizeof(*self->ptr));

        if (nullptr == self->ptr) {
            print_allocation_failure_message();
            exit(EXIT_FAILURE);
        }
    } else if (self->len == self->cap) {
        auto const old_cap = self->cap;
        self->cap = (String_GROWTH_NUMERATOR * self->cap + String_GROWTH_SHIFT) / String_GROWTH_DENOMINATOR;
        self->ptr = realloc(self->ptr, sizeof(*self->ptr) * self->cap);

        if (nullptr == self->ptr) {
            print_allocation_failure_message();
            exit(EXIT_FAILURE);
        }

        memset(self->ptr + old_cap, 0, sizeof(*self->ptr) * (self->cap - old_cap));
    }

    self->ptr[self->len] = value;
    self->len += 1;
}

void
String_append(String *self, Str str)
{
    for (size_t i = 0; i < str.len; ++i) {
        String_push(self, str.ptr[i]);
    }
}

typedef struct VecStr
{
    Str *ptr;
    size_t len;
    size_t cap;
} VecStr;

#define VecStr_DEFAULT ((VecStr){.ptr = nullptr, .len = 0, .cap = 0})

enum
{
    VecStr_INITIAL_CAP = 2,
    VecStr_GROWTH_NUMERATOR = 3,
    VecStr_GROWTH_DENOMINATOR = 2,
    VecStr_GROWTH_SHIFT = 1,
};

void
VecStr_push(VecStr *self, Str str)
{
    if (0 == self->cap) {
        self->cap = VecStr_INITIAL_CAP;
        self->ptr = calloc(self->cap, sizeof(*self->ptr));

        if (nullptr == self->ptr) {
            print_allocation_failure_message();
            exit(EXIT_FAILURE);
        }
    } else if (self->len == self->cap) {
        self->cap = (VecStr_GROWTH_NUMERATOR * self->cap + VecStr_GROWTH_SHIFT) / VecStr_GROWTH_DENOMINATOR;
        self->ptr = realloc(self->ptr, sizeof(*self->ptr) * self->cap);

        if (nullptr == self->ptr) {
            print_allocation_failure_message();
            exit(EXIT_FAILURE);
        }
    }

    self->ptr[self->len] = str;
    self->len += 1;
}

Str
VecStr_pop(VecStr *self)
{
    if (0 == self->len) {
        fprintf(stderr, "cannot pop an empty Vec\n");
        exit(EXIT_FAILURE);
    }

    self->len -= 1;
    return self->ptr[self->len];
}

void
VecStr_drop(VecStr *self)
{
    free(self->ptr);
    self->ptr = nullptr;
    self->len = self->cap = 0;
}

VecStr
construct_path_stack_from_path(Str path)
{
    auto result = VecStr_DEFAULT;

    if (0 == path.len) {
        return result;
    }

    // Skip root directory
    if ('/' == path.ptr[0]) {
        path = Str_slice(path, 1, path.len);
    }

    while (0 != path.len) {
        auto segment = Str_split_once(&path, '/');

        if (0 == segment.len && 0 != path.len) {
            segment = path;
            path = Str_DEFAULT;
        }

        if (Str_eq(segment, Str("."))) {
            continue;
        } else if (Str_eq(segment, Str(".."))) {
            if (0 == result.len || Str_eq(result.ptr[result.len - 1], Str(".."))) {
                VecStr_push(&result, segment);
            } else {
                VecStr_pop(&result);
            }
        } else {
            VecStr_push(&result, segment);
        }
    }

    return result;
}

void
Str_print(Str self)
{
    for (size_t i = 0; i < self.len; ++i) {
        putchar(self.ptr[i]);
    }
}

char *
get_dot_string()
{
    char *result = calloc(sizeof("."), sizeof(*result));

    if (nullptr == result) {
        print_allocation_failure_message();
        exit(EXIT_FAILURE);
    }

    result[0] = '.';

    return result;
}

char *
relativize_path(const char *c_path_source, const char *c_path_destination)
{
    auto const path_source = Str_from_c(c_path_source);
    auto const path_destination = Str_from_c(c_path_destination);

    auto stack_source = construct_path_stack_from_path(path_source);
    auto stack_destination = construct_path_stack_from_path(path_destination);

    if (0 == stack_source.len && 0 == stack_destination.len) {
        return get_dot_string();
    }

    size_t overlap_len = 0;

    for (size_t i = 0; i < stack_source.len && i < stack_destination.len; ++i) {
        if (!Str_eq(stack_source.ptr[i], stack_destination.ptr[i])) {
            break;
        }

        overlap_len += 1;
    }

    if (overlap_len == stack_destination.len && overlap_len + 1 == stack_source.len) {
        return get_dot_string();
    }

    auto result_string = String_DEFAULT;

    // Remove last file from source stack
    if (0 != stack_source.len) {
        auto const source_last = VecStr_pop(&stack_source);

        if (overlap_len >= stack_source.len) {
            String_append(&result_string, source_last);

            if (overlap_len < stack_destination.len) {
                String_push(&result_string, '/');
            }
        }
    }

    for (size_t i = overlap_len; i < stack_source.len; ++i) {
        String_append(&result_string, Str(".."));

        if (i + 1 != stack_source.len || overlap_len < stack_destination.len) {
            String_push(&result_string, '/');
        }
    }

    for (size_t i = overlap_len; i < stack_destination.len; ++i) {
        String_append(&result_string, stack_destination.ptr[i]);

        if (i + 1 != stack_destination.len) {
            String_push(&result_string, '/');
        }
    }

    VecStr_drop(&stack_destination);
    VecStr_drop(&stack_source);

    return result_string.ptr;
}

// int
// main(int argc, char *argv[])
// {
//     if (argc < 3) {
//         return 1;
//     }

//     printf("%s\n", relativize_path(argv[1], argv[2]));
// }
