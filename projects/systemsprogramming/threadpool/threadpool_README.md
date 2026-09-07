# threadpool

A fixed-size thread pool built with raw pthreads — 4 worker threads, a shared job queue, one mutex, one condition variable. Later reused inside `jashserver` to handle connections concurrently instead of one-at-a-time.

## Why `experiments/` exists

This wasn't built by writing the final thread pool and being done. Each file in `experiments/` is an earlier, smaller step:

- `first_thread.cpp` — just spinning up a single pthread, checking I actually understand the create/join API before doing anything concurrent.
- `race_condition.cpp` — deliberately unsynchronized shared state across threads, to actually watch a race condition happen instead of just reading about one.
- `mutex_test` — the same shared state, now protected, to see the difference directly.
- `condition_variable.cpp` — a producer/consumer setup using a condition variable instead of busy-waiting or polling.
- `job_queue.cpp` — a queue of callable jobs shared across threads, the piece right before "thread pool" clicks into being one thing.
- `thread_pool.cpp` — the actual pool: fixed worker threads pulling from a shared `std::queue<std::function<void()>>`, blocking on a condition variable when the queue is empty, woken by `pthread_cond_signal` on submit.

## How shutdown works

Destructor sets a `shutdown` flag under the lock, broadcasts on the condition variable so every waiting worker wakes up, and joins all threads. Workers check `shutdown && jobs.empty()` after waking — so a worker won't exit early if there's still a queued job left to drain, but it also won't wait forever once shutdown is signaled and the queue is empty. This is the "poison pill via flag + broadcast" pattern rather than pushing an actual sentinel job into the queue.

## What I learned building this that isn't obvious from a diagram

The order of operations around the condition variable matters more than it looks: `pthread_cond_wait` has to be called while holding the lock, and it has to be in a `while` loop checking the actual condition, not an `if` — spurious wakeups are real, and it's easy to write code that looks correct and only breaks intermittently.

## What's not done yet

Pool size is hardcoded (4 threads), no way to wait on a submitted job's result (no future/promise), and no backpressure if jobs are submitted faster than they're consumed. Fine for now since the point was understanding the synchronization primitives, not building a production scheduler.
