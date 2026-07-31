


#include "shell.hpp"
#include <iostream>



Shell::Shell(){

}

void Shell::run(){
    
    std::string input;
    Parser parser;
    Executor executor;
    Builtins builtins;
    Pipeline pipeline;
    while(true){
        std::cout<< "JashShell > ";
        if(!std::getline(std::cin, input)){
            break;
        }
        //built in exit command
        if(input=="exit"){
            break;
        }
        pipeline=parser.parse(input);
        if (pipeline.commands.empty()){
            continue;
        }

        if (!builtins.execute(pipeline)){
            executor.execute(pipeline);
        }

    }
    
    std::cout<<"You terminated Shell\n";
   
}