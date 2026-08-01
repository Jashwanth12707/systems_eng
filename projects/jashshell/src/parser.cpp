#include "parser.hpp"

#include <sstream>

Parser::Parser()
{

}

Pipeline Parser::parse(const std::string& input)
{
    Pipeline pipeline;

    std::istringstream ss(input);

    std::string word;

    Command command;

    bool first = true;

    while (ss >> word)
    {
        if (word == "|")
        {
            // Save the command we just finished
            if (!command.program.empty())
            {
                pipeline.commands.push_back(command);
            }

            // Start a fresh command
            command = Command();
            first = true;

            continue;
        }

        else if (first)
        {
            command.program = word;
            first = false;
        }
        else if(word == "<")
        {
            ss >> command.inputFile;
        }
        else if(word == ">")
        {
            ss >> command.outputFile;
            command.append = false;
        }
        else if(word == ">>")
        {
            ss >> command.outputFile;
            command.append = true;
        }
        else if(word == "&")
        {
            pipeline.background = true;
        }  
        else
        {
            command.arguments.push_back(word);
        }
    }

    // Push the last command
    if (!command.program.empty())
    {
        pipeline.commands.push_back(command);
    }

    return pipeline;
}