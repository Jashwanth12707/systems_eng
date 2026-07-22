#include <fcntl.h>
#include <unistd.h>
#include <iostream>

int main()
{
    int fd = open("test.bin",
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0644);

    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    write(fd, "ABCDE", 5);

    lseek(fd, 10, SEEK_SET);

    write(fd, "X", 1);

    close(fd);

    return 0;
}