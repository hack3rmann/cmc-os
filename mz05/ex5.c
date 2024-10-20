#include <stdlib.h>
#include <stdio.h>

typedef enum DirectoryType
{
    DIRECTORY_TYPE_CURRENT,
    DIRECTORY_TYPE_PARENT,
    DIRECTORY_TYPE_CHILD,
} DirectoryType;

DirectoryType
DirectoryType_of(char const *path, size_t len)
{
    if (1 == len && '.' == path[0]) {
        return DIRECTORY_TYPE_CURRENT;
    } else if (2 == len && '.' == path[0] && '.' == path[1]) {
        return DIRECTORY_TYPE_PARENT;
    } else {
        return DIRECTORY_TYPE_CHILD;
    }
}

char const *
next_path_segment(char const *path)
{
    if (nullptr == path) {
        return nullptr;
    }

    if ('/' == *path) {
        path += 1;
    }

    while ('\0' != *path && '/' != *path) {
        path += 1;
    }

    if ('/' == *path) {
        path += 1;
    }

    return path;
}

char const *
child_path(char const* full, char const* parent, char const *cur)
{
    if (nullptr == cur) {
        return nullptr;
    }

    auto const next_segment = next_path_segment(cur);

    if ('\0' == *next_segment) {
        return next_segment;
    }

    auto const segment_len = next_segment - cur - 1;
    auto const segment_type = DirectoryType_of(next_segment, segment_len);

    switch (segment_type) {
    case DIRECTORY_TYPE_CHILD:
        return next_segment;
    case DIRECTORY_TYPE_CURRENT:
        return child_path(full, parent, next_segment);
    case DIRECTORY_TYPE_PARENT:
        return next_segment;
    }

    return cur;
}

int
main(int argc, char *argv[])
{
    if (argc > 1) {
        // printf("%s\n", child_path(argv[1]));
    }
}
