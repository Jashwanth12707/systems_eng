#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include "pipeline.hpp"

class Executor
{
public:

    Executor();

    void execute(const Pipeline& pipeline);

   

private:

    void executeSimpleCommand(const Command& command,bool background);
    void executePipeline(const Pipeline& pipeline);
    void applyRedirections(const Command& command);
};

#endif