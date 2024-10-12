#include <stdlib.h>

typedef enum PathType
{
    DIRECTORY_TYPE_CURRENT,
    DIRECTORY_TYPE_PARENT,
    DIRECTORY_TYPE_CHILD,
} DirectoryType;

DirectoryType
DirectoryType_of(char const* path, size_t len)
{

}

char const*
next_path_segment(char const* path)
{
    if (nullptr == path) {
        return nullptr;
    }

    while ('\0' != *path && '\\' != *path) {
        path += 1;
    }

    if ('\\' == *path) {
        path += 1;
    }

    return path;
}

char const*
child_path(char const* path)
{
    if (nullptr == path) {
        return nullptr;
    }

    auto const next_segment = next_path_segment(path);
    
    if ('\0' == *next_segment) {
        return next_segment;
    }

    auto const segment_len = next_segment - path - 1;
}
