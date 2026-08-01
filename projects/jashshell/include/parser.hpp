#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include "pipeline.hpp"


class Parser
{
public:
    Parser();

    // Parses a command line into individual tokens.
    Pipeline parse(const std::string& input);
};

#endif
