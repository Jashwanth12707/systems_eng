#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <cctype>

using namespace std;

int main(int argc,char*argv[]){

    if (argc!=2){
        cerr<<"Usage : wc <filename\n";
        return 1;
    }
    
    int fd=open(argv[1],O_RDONLY);
    
    if( fd ==-1){
        perror("open");
        return 1;
    }
    char buff[4096];
    ssize_t buffsize;
    int bytes=0;
    int lines=0;
    int words=0;
    bool inword=false;

    while((buffsize = read(fd, buff, sizeof(buff))) > 0){
        bytes+=buffsize;

        for(int i=0;i<buffsize;i++){

            char ch =buff[i];

            if (isspace(static_cast<unsigned char>(ch)))
            {
                inword = false;
            }
            else if (!inword)
            {
                words++;
                inword = true;
            }
            if(ch=='\n'){
                lines++;
            }
        }
    }
    close(fd);
cout<<lines<<" "
    <<words<<" "
    <<bytes<<"\n";



}