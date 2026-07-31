#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>


class Parser
{
public:
    Parser();

    // Parses a command line into individual tokens.
    std::vector<std::string> parse(const std::string& input);
};

#endif