#ifndef BUILTINS_HPP
#define BUILTINS_HPP

#include "pipeline.hpp"
#include "command.hpp"

class Builtins
{
public:
    Builtins();

    bool execute(Pipeline&pipeline);
};

#endif