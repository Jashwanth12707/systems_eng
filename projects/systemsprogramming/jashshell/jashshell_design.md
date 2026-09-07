# Design notes

A few decisions worth writing down, mostly so I remember why later.

**Parsing is single-pass, not tokenize-then-parse.** The parser walks
the input word by word and decides what each word means as it goes
(program name, pipe separator, redirect target, argument) instead of
building a token list first and running a separate parse step over
it. Simpler for a shell this size, and it's the same general shape
real shells use for a first pass. Downside: no quoting support yet,
so `"two words"` as one argument doesn't work.

**Builtins are checked before the executor, not inside it.** `cd`
has to run in the shell's own process — if you `fork()` first and
`chdir()` in the child, the child's directory changes but the
parent shell's doesn't, and the cd silently does nothing useful.
That's an easy mistake to make once and never forget after.

**Closing unused pipe fds is not optional.** Early version of the
pipeline code left extra pipe fds open in children that didn't need
them. Everything looked fine for a 2-command pipeline and then
hung on 3+, because a child holding an unused write end kept a
downstream reader waiting for EOF that was never going to come.
Fixed by explicitly closing every pipe fd in a child except the
two it's actually using.

**Reaping is signal-driven, not polled.** Background jobs get
cleaned up in a SIGCHLD handler calling waitpid with WNOHANG,
rather than checking for finished children on some timer or on
every loop iteration. Means zombies get reaped as soon as the
kernel tells us, not on the next arbitrary check.

**What I'd change if I rebuilt this:** proper tokenizing with quote
support, and job control (fg/bg/jobs) instead of fire-and-forget
backgrounding. Left as-is for now since the roadmap says move
forward once the mechanism is understood, not once every feature
is added.
