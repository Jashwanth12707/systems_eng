#ifndef SHELL_HPP
#define SHELL_HPP

#include <string>
#include <vector>

#include "parser.hpp"
#include "executor.hpp"
#include "builtins.hpp"

class Shell
{
public:
    Shell();
    void run();
};

#endif