#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
using namespace std;



int main(int argc,char*argv[]){

    if(argc!=2){
        cout<<"Usage: ./mycat <filename>\n";
        return 1 ;
    }

    int fd=open(argv[1],O_RDONLY);
    
    if( fd ==-1){
        perror("open");
        return 1;
    }

    
char buff[4096];

ssize_t buffsize = read(src_fd, buff, sizeof(buff));

while (buffsize > 0)
{
    write(dest_fd, buff, buffsize);
    buffsize = read(src_fd, buff, sizeof(buff));
}

    
    return 0;

}