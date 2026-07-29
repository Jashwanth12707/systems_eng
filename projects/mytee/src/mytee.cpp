#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <cerrno>

using namespace std;

int main(int argc,char*argv[]){

    if(argc!=2){
        cerr<<"Usage: ./mytee output_file\n";
        return 1;
    }

    int dest_fd=open(argv[1],O_WRONLY | O_CREAT | O_TRUNC,0644);
    if(dest_fd==-1){
        perror("open");
        return 1;
    }

    char buff[4096];
    ssize_t buffsize;
    
    while ((buffsize = read(STDIN_FILENO, buff, sizeof(buff))) > 0)
    {   
        write(STDOUT_FILENO,buff,buffsize);
        write(dest_fd, buff, buffsize);
    }
    close(dest_fd);

}