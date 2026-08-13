#include "builtins.hpp"

#include <iostream>
#include <unistd.h>

Builtins::Builtins()
{

}

bool Builtins::execute(Pipeline& pipeline)
{
    if (pipeline.commands.empty())
    {
        return true;
    }

    Command& command = pipeline.commands[0];

    if (command.program == "cd")
    {
        if (command.arguments.empty())
        {
            std::cout << "Usage: cd <directory>\n";
        }
        else
        {
            if (chdir(command.arguments[0].c_str()) != 0)
            {
                perror("cd");
            }
        }

        return true;
    }

    return false;
}