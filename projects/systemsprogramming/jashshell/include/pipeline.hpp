#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <vector>

#include "command.hpp"

struct Pipeline
{
    std::vector<Command> commands;

    bool background = false;
};

#endif