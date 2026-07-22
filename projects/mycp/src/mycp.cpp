#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
using namespace std;

int main(int argc,char*argv[]){

    if(argc!=3){
        cerr<<"Usage: ./mycp <source> <destination>\n";
        return 1;
    }

    int src_fd=open(argv[1],O_RDONLY);
    if(src_fd==-1){
        perror("open");
        return 1;
    }
    int dest_fd=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0644);
    if(dest_fd==-1){
        perror("open");
        close(src_fd);
        return 1;  
    }
    
    char buff[4096];
    ssize_t buffsize;

    while ((buffsize = read(src_fd, buff, sizeof(buff))) > 0)
    {
        write(dest_fd, buff, buffsize);
    }
    close(src_fd);
    close(dest_fd);

}