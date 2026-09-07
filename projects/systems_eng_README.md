# systems_eng

This is where I'm working through systems programming from the ground up — not to collect finished projects, but to actually understand what's happening underneath normal software: processes, memory, concurrency, networking, and eventually embedded/real-time stuff.

The rule I'm following: build the mechanism myself before I'm allowed to move on. No skipping to the next thing just because the current one "works."

## What's in here

All the code lives under `projects/systemsprogramming/`.

- **mycat / mycp / mytee / mywc** — small reimplementations of core Linux utilities. The point wasn't to clone `cat` or `cp` exactly, it was to actually use `open()`, `read()`, `write()`, `close()` directly instead of treating file I/O as something that just happens.
- **jashshell** — a Unix shell written from scratch. Handles piping across any number of commands (not just two), input/output redirection (`<`, `>`, `>>`), background jobs with `&`, and reaps zombie processes via `SIGCHLD` instead of leaking them.
- **threadpool** — a fixed-size thread pool built with raw pthreads, mutexes, and condition variables. The `experiments/` folder is intentional — it's where I broke things on purpose (race conditions, deadlock attempts, job queue stress) before writing the "real" version.
- **jashserver** — a multithreaded HTTP server over raw sockets, using the thread pool above to handle connections concurrently. Serves static files and a couple of live routes (there's an `/imu` endpoint that returns a mutex-protected IMU struct — earliest attempt at treating a server like a live sensor endpoint, not just a file server).

## Why it's structured this way

Each folder is meant to stand alone — you should be able to go into `jashshell/` and understand it without needing the other projects. But they build on each other conceptually: the thread pool shows up again inside the HTTP server, `fork`/`exec` from the shell comes back later for process management ideas, etc.

## Status

CLI utils and jashshell are done. Thread pool and jashserver are functional but still getting cleaned up. Next up is a proper memory allocator, then moving into persistence (a small KV store) before I get to embedded/RTOS stuff.

Longer term this is heading toward robotics/aerospace software — sensor fusion, flight controllers, autonomy — which is why some of the demo code (the IMU route, the `sr71_manual.pdf` thread pool example) already has that flavor even at this early, unglamorous stage.
