#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>

void
multiply(void *ptr1, void *ptr2, void *ptr_result)
{
}

int
main(int argc, char *argv[])
{
    if (argc < 4) {
        return 1;
    }

    auto const path1 = argv[1];
    auto const path2 = argv[2];
    auto const path_result = argv[3];

    auto const file_desc1 = open(path1, O_RDONLY);
    auto const file_desc2 = open(path2, O_RDONLY);
    auto const file_desc_result = creat(path_result, 0600);

    auto file_stat = (struct stat){};

    if (!fstat(file_desc1, &file_stat)) {
        exit(EXIT_FAILURE);
    }

    auto const file1_size = file_stat.st_size;

    if (!fstat(file_desc2, &file_stat)) {
        exit(EXIT_FAILURE);
    }

    auto const file2_size = file_stat.st_size;
    auto const file_result_size = file1_size + file2_size;

    auto ptr1 = mmap(nullptr, file1_size, PROT_READ, 0, file_desc1, 0);
    auto ptr2 = mmap(nullptr, file2_size, PROT_READ, 0, file_desc2, 0);
    auto ptr_result = mmap(nullptr, file1_size + file2_size, PROT_WRITE, 0, file_desc_result, 0);

    close(file_desc_result);
    close(file_desc2);
    close(file_desc1);

    multiply(ptr1, ptr2, ptr_result);

    munmap(ptr_result, file_result_size);
    munmap(ptr2, file2_size);
    munmap(ptr1, file1_size);
}
