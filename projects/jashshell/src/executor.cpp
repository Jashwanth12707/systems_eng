#include "executor.hpp"

#include <cstdlib>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

Executor::Executor()
{

}

void Executor::execute(const Pipeline& pipeline)
{
    if (pipeline.commands.empty())
    {
        return;
    }

    if (pipeline.commands.size() == 1)
    {
        executeSimpleCommand(pipeline.commands[0]);
    }
    else
    {
        executePipeline(pipeline);
    }
}

void Executor::executeSimpleCommand(const Command& command)
{
    std::vector<const char*> argv;

    argv.push_back(command.program.c_str());

    for (const auto& arg : command.arguments)
    {
        argv.push_back(arg.c_str());
    }

    argv.push_back(nullptr);

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return;
    }
    else if (pid == 0)
    {
        execvp(argv[0], const_cast<char* const*>(argv.data()));

        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else
    {
        waitpid(pid, nullptr, 0);
    }
}

void Executor::executePipeline(const Pipeline& pipeline)
{
    std::cout << "Pipeline execution not implemented yet.\n";

    // This function will later:
    //
    // 1. Create pipes
    // 2. Fork for every command
    // 3. Connect stdin/stdout using dup2()
    // 4. Execute every command
    // 5. Wait for all children
}