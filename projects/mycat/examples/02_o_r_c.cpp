#include <fcntl.h>      // Declares open() and file access flags
#include <unistd.h>     // Declares read(), write(), close()
#include <iostream>     // Provides std::cout

int main()
{
    // Ask the kernel to open "notes.txt" in read-only mode.
    // On success, the kernel returns a file descriptor (e.g., 3).
    int fd = open("../notes.txt", O_RDONLY);

    // If open() fails, it returns -1.
    if (fd == -1)
    {
        std::cout << "Failed to open file\n";
        return 1;
    }

    // Allocate a 100-byte buffer in RAM.
    char buffer[100];

    // Ask the kernel to copy up to 100 bytes from the file into the buffer.
    // bytesRead stores the number of bytes actually copied.
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));

    // Print only the valid bytes that were read.
    std::cout.write(buffer, bytesRead);

    // Tell the kernel we're finished with this file.
    close(fd);

    // Exit successfully.
    return 0;
}