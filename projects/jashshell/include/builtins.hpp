#ifndef BUILTINS_HPP
#define BUILTINS_HPP

#include <vector>
#include <string>

class Builtins
{
public:
    Builtins();

    bool execute(const std::vector<std::string>& tokens);
};

#endif