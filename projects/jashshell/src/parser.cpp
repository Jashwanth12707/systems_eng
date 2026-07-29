#include "parser.hpp"
#include <sstream>



//constructor
Parser::Parser(){

}

//parser function using sstream where the ss takes the string and 
//put that into tokens array 
//return the array
std::vector<std::string> Parser::parse(const std::string&input){
    std::istringstream ss(input);
    std::vector<std::string>tokens;
    std::string word;
    while(ss >> word){
        tokens.push_back(word);
    }
    return tokens;
}

