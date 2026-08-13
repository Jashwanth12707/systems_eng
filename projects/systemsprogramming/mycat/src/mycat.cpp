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
    ssize_t buffsize=read(fd,buff,4096);


    while (buffsize>0){
        write(STDOUT_FILENO,buff,buffsize);
        buffsize=read(fd,buff,sizeof(buff));
    }

    close(fd);
 
    
    return 0;

}