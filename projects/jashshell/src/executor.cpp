#include "executor.hpp"

#include <cstdlib>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <array>
#include <fcntl.h>
#include <signal.h>


void handleSigchld(int)
{
    while (waitpid(-1, nullptr, WNOHANG) > 0)
    {
        // Keep reaping every finished child.
    }
}

Executor::Executor()
{
signal(SIGCHLD, handleSigchld);
signal(SIGINT, SIG_IGN); // Ignore SIGINT in the parent process
}


void Executor::execute(const Pipeline& pipeline)
{
    if (pipeline.commands.empty())
    {
        return;
    }

    if (pipeline.commands.size() == 1)
    {
        executeSimpleCommand(pipeline.commands[0], pipeline.background);
    }
    else
    {
        executePipeline(pipeline);
    }
}

void Executor::executeSimpleCommand(const Command& command,bool background)
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
    {   // fork() inherited SIGINT=IGNORE from JashShell.
        // Undo that for the command.
        signal(SIGINT, SIG_DFL);
        applyRedirections(command);
        execvp(argv[0], const_cast<char* const*>(argv.data()));

        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else
    {   if(!background)
        {
            waitpid(pid, nullptr, 0);
        }
    }
}

    //. Create pipes
    // 2. Fork for every command
    // 3. Connect stdin/stdout using dup2()
    // 4. Execute every command
    // 5. Wait for all children
void Executor::executePipeline(const Pipeline& pipeline)
{
    // Total number of commands in the pipeline.
    //
    // Example:
    // ls | grep cpp | wc
    //
    // numCommands = 3
    size_t numCommands = pipeline.commands.size();

    // Number of pipes is always one less than the number of commands.
    //
    // command1 | command2 | command3
    //          ^          ^
    //        Pipe0      Pipe1
    //
    // Therefore:
    // numPipes = numCommands - 1
    size_t numPipes = numCommands - 1;

    // ---------------------------------------------------------
    // Create storage for every pipe.
    //
    // Each pipe consists of two file descriptors:
    //
    // pipes[i][0] -> Read end
    // pipes[i][1] -> Write end
    //
    // Example:
    //
    // Pipe0 = {3,4}
    // Pipe1 = {5,6}
    // ---------------------------------------------------------
    std::vector<std::array<int, 2>> pipes(numPipes);

    // Actually ask the kernel to create every pipe.
    //
    // pipe() fills the array with two file descriptors.
    //
    // Before:
    // pipes[0] = {?, ?}
    //
    // After:
    // pipes[0] = {3,4}
    //
    // Now the kernel has allocated a communication channel.
    for (size_t i = 0; i < numPipes; i++)
    {
        if (pipe(pipes[i].data()) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Store every child PID so the parent can wait later.
    std::vector<pid_t> pids;

    // ---------------------------------------------------------
    // One iteration == One command == One child process
    //
    // Example:
    //
    // ls | grep cpp | wc
    //
    // i=0 -> child running ls
    // i=1 -> child running grep
    // i=2 -> child running wc
    // ---------------------------------------------------------
    for (size_t i = 0; i < numCommands; i++)
    {
        // Duplicate the current process.
        //
        // After fork():
        //
        // Parent
        // Child
        //
        // Both continue executing from the next line.
        pid_t pid = fork();

        if (pid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        // -----------------------------------------------------
        // CHILD PROCESS
        // -----------------------------------------------------
        if (pid == 0)
        {   
            signal(SIGINT, SIG_DFL);
            // -------------------------------------------------
            // CONNECT INPUT
            //
            // Every command except the first receives input
            // from the previous pipe.
            //
            // Example:
            //
            // ls | grep | wc
            //
            // grep reads from Pipe0
            // wc    reads from Pipe1
            //
            // First command has no previous pipe,
            // so we skip it.
            // -------------------------------------------------
            if (i > 0)
            {
                // Replace stdin with the previous pipe's read end.
                //
                // After dup2:
                //
                // STDIN -----> Pipe Read End
                //
                // So every read() performed by the program
                // actually reads from the pipe.
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // -------------------------------------------------
            // CONNECT OUTPUT
            //
            // Every command except the last sends its output
            // into the next pipe.
            //
            // Example:
            //
            // ls | grep | wc
            //
            // ls    writes to Pipe0
            // grep  writes to Pipe1
            // wc    writes to terminal
            // -------------------------------------------------
            if (i < numCommands - 1)
            {
                // Replace stdout with the pipe's write end.
                //
                // After dup2:
                //
                // STDOUT -----> Pipe Write End
                //
                // Any printf(), cout, write(), etc.
                // automatically goes into the pipe.
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            //override stdin/stdout if redirection is specified

            applyRedirections(pipeline.commands[i]);

            // -------------------------------------------------
            // Close EVERY original pipe descriptor.
            //
            // Why?
            //
            // dup2() already copied the required pipe into
            // stdin/stdout.
            //
            // We no longer need these original descriptors.
            //
            // Leaving them open causes resource leaks and,
            // more importantly, prevents readers from ever
            // seeing EOF because extra write descriptors stay
            // open.
            //
            // After this:
            //
            // stdin/stdout still work.
            // Only the duplicate descriptors are closed.
            // -------------------------------------------------
            for (size_t j = 0; j < numPipes; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // -------------------------------------------------
            // Build argv for execvp().
            //
            // execvp() expects:
            //
            // {"grep","cpp",nullptr}
            //
            // Example:
            //
            // grep cpp
            //
            // argv[0] = "grep"
            // argv[1] = "cpp"
            // argv[2] = nullptr
            // -------------------------------------------------
            std::vector<const char*> argv;

            // Program name.
            argv.push_back(pipeline.commands[i].program.c_str());

            // Every argument.
            for (const auto& arg : pipeline.commands[i].arguments)
            {
                argv.push_back(arg.c_str());
            }

            // execvp() requires a nullptr terminator.
            argv.push_back(nullptr);

            // -------------------------------------------------
            // Replace this child process with the actual command.
            //
            // Before:
            //
            // Child executing executePipeline()
            //
            // After:
            //
            // Child executing /bin/ls
            //
            // or
            //
            // Child executing /bin/grep
            //
            // or
            //
            // Child executing /bin/wc
            //
            // File descriptor connections created using dup2()
            // remain intact.
            // -------------------------------------------------
            execvp(argv[0], const_cast<char* const*>(argv.data()));

            // execvp() only returns if it fails.
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        // -----------------------------------------------------
        // PARENT PROCESS
        //
        // Parent never executes execvp().
        //
        // It simply remembers every child's PID so that
        // it can wait for them later.
        //
        // Then the loop continues and creates the next child.
        // -----------------------------------------------------
        pids.push_back(pid);
    }

    // ---------------------------------------------------------
    // All children have been created.
    //
    // Parent no longer needs ANY pipe.
    //
    // If the parent keeps write ends open,
    // readers never receive EOF.
    //
    // So close every pipe in the parent.
    // ---------------------------------------------------------
    for (size_t i = 0; i < numPipes; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // ---------------------------------------------------------
    // Finally wait for every child to finish.
    //
    // We wait AFTER creating all children because
    // pipelines must run simultaneously.
    //
    // Waiting inside the fork loop would execute
    // commands one-by-one instead of concurrently.
    // ---------------------------------------------------------
    if(!pipeline.background)
    {
        for (pid_t pid : pids)
        {
            waitpid(pid, nullptr, 0);
        }
    }
}
    void Executor::applyRedirections(const Command& command)
{
    // <
    if (!command.inputFile.empty())
    {
        int fd = open(command.inputFile.c_str(), O_RDONLY);

        if (fd == -1)
        {
            perror("open");
            exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDIN_FILENO) == -1)
        {
            perror("dup2");
            close(fd);
            exit(EXIT_FAILURE);
        }

        close(fd);
    }

    // > and >>
    if (!command.outputFile.empty())
    {
        int flags = O_WRONLY | O_CREAT;

        if (command.append)
        {
            flags |= O_APPEND;   // >>
        }
        else
        {
            flags |= O_TRUNC;    // >
        }

        int fd = open(
            command.outputFile.c_str(),
            flags,
            0644
        );

        if (fd == -1)
        {
            perror("open");
            exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(fd);
            exit(EXIT_FAILURE);
        }

        close(fd);
    }
}