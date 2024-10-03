#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

typedef int FileDesc;

enum
{
    LSEEK_ERROR_RESULT = -1,
    READ_ERROR_RESULT = -1,
    N_BYTE_BITS = 8,
};

int32_t
i32_read(FileDesc file_desc)
{
    uint8_t single_byte = 0;
    uint32_t result_unsigned = 0;

    for (size_t i = 0; i < sizeof(result_unsigned); ++i) {
        if (READ_ERROR_RESULT == read(file_desc, &single_byte, sizeof(single_byte))) {
            fprintf(stderr, "`read` failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        result_unsigned <<= N_BYTE_BITS;
        result_unsigned |= (uint32_t) single_byte;
    }

    return (int32_t) result_unsigned;
}

typedef struct Node
{
    int32_t key;
    int32_t left_index;
    int32_t right_index;
} Node;

Node
Node_read_from_file(FileDesc file_desc, size_t index)
{
    if (-1 == lseek(file_desc, sizeof(Node) * index, SEEK_SET)) {
        fprintf(stderr, "`lseek` failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    Node const result = {
        .key = i32_read(file_desc),
        .left_index = i32_read(file_desc),
        .right_index = i32_read(file_desc),
    };

    return result;
}

void
Node_print_tree_from_file_recursive(FileDesc file_desc, size_t index)
{
    auto const cur_node = Node_read_from_file(file_desc, index);

    if (0 != cur_node.right_index) {
        Node_print_tree_from_file_recursive(file_desc, cur_node.right_index);
    }

    printf("%d ", cur_node.key);

    if (0 != cur_node.left_index) {
        Node_print_tree_from_file_recursive(file_desc, cur_node.left_index);
    }
}

void
Node_print_tree_from_file(FileDesc file_desc)
{
    Node_print_tree_from_file_recursive(file_desc, 0);
}

enum
{
    FILE_NAME_ARG_INDEX = 1,
    N_ARGS = FILE_NAME_ARG_INDEX,
    OPEN_ERROR_VALUE = -1,
};

int
main(int argc, char *argv[])
{
    if (N_ARGS != argc) {
        fprintf(stderr, "%zu arguments are required\n", (size_t) N_ARGS);
        exit(EXIT_FAILURE);
    }

    auto const file_name = argv[FILE_NAME_ARG_INDEX];

    FileDesc const file_desc = open(file_name, O_RDONLY);

    if (OPEN_ERROR_VALUE == file_desc) {
        fprintf(stderr, "failed to open file '%s': %s\n", file_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (0 != close(file_desc)) {
        fprintf(stderr, "failed to close a file with descriptor #%d: %s\n", file_desc, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
