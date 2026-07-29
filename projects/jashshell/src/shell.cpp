#include "shell.hpp"
#include <iostream>



Shell::Shell(){

}

void Shell::run(){
    std::string input;
    Parser parser;
    std::vector<std::string>parsed;
    while(true){
        std::cout<< "JashShell > ";
        if(std::getline(std::cin, input)){
            break;
        }
        //built in exit command
        if(input=="exit"){
            break;
        }
        parsed=parser.parse(input);
       
    }
    
    std::cout<<"You terminated Shell\n";
   
}