#include <fcntl.h>
#include <unistd.h>
#include <iostream>

int main(){
    int fd=open("notes.txt",O_RDONLY);

    if( fd ==-1){
        std::cout<<"failed to open file\n";
        return 1;
    }

    char buffer[20]={0};
    ssize_t bytesRead =read(fd,buffer,20);
    std::cout<< "First read: "
            <<buffer
            <<" ("<<bytesRead <<" bytes)\n";
    
            bytesRead = read(fd, buffer, 20);

    std::cout << "Second read: "
            << buffer
            << " (" << bytesRead << " bytes)\n";

    std::cout << "Third read: "
            <<buffer
            <<" ("<< bytesRead <<" bytes)\n";

    close(fd);

    return 0;
}