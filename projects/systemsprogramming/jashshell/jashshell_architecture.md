# Architecture

Input line
   |
   v
Parser::parse()          -> splits on '|', pulls out '<' '>' '>>' '&'
   |
   v
Pipeline { commands[], background }
   |
   +--> Builtins::execute()   (cd, exit — handled in-process, no fork)
   |
   +--> Executor::execute()
           |
           +-- single command --> executeSimpleCommand()
           |         fork() -> execvp() in child, waitpid() in parent
           |         (skipped if backgrounded)
           |
           +-- multiple commands --> executePipeline()
                     1. create N-1 pipes
                     2. fork() once per command
                     3. in each child: dup2() stdin/stdout to the
                        correct pipe ends, close every unused fd,
                        then execvp()
                     4. parent closes all pipe fds, waits on every
                        child unless the pipeline is backgrounded

Background reaping happens independently: a SIGCHLD handler calls
waitpid(-1, ..., WNOHANG) in a loop, so finished background jobs
get cleaned up whenever the signal fires rather than blocking the
main read loop.

## Process tree for a 3-stage pipeline

```
JashShell (parent)
  |
  +-- cmd1  (stdout -> pipe0 write end)
  +-- cmd2  (stdin <- pipe0 read end, stdout -> pipe1 write end)
  +-- cmd3  (stdin <- pipe1 read end)
```

Every fd not needed by a given child gets closed in that child
before exec — that's the part that isn't obvious until you get it
wrong and something hangs.
