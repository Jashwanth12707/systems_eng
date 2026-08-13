#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>
struct Command{
    //executable
    std::string program;
    //argv
    std::vector<std::string>arguments;

    //redirection
    std::string inputFile;
    std::string outputFile;

    bool append =false;

    //background
    bool background=false;

};


#endif