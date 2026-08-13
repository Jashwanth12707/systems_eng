#include <fcntl.h>
#include <unistd.h>
#include <iostream>

int main()
{
    int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1)
    {
        std::cout << "Open failed\n";
        return 1;
    }

    char msg[] = "Hello Linux!";

    ssize_t bytesWritten = write(fd, msg, 12);

    std::cout << "Bytes written: " << bytesWritten << std::endl;

    close(fd);

    return 0;
}