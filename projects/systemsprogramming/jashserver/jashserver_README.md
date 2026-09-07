# jashserver

A multithreaded HTTP/1.1 server built directly on raw sockets — no framework, no libevent, just `socket()`/`bind()`/`listen()`/`accept()` and a thread pool underneath to handle connections concurrently.

## What it does

- Accepts TCP connections and parses raw HTTP requests
- Serves static files out of `public/` with content-type detection based on extension
- A handful of live routes on top of static serving:
  - `/profile`, `/judge`, `/fibonacci` — small dynamic endpoints
  - `/imu` — returns the latest IMU reading (roll/pitch/yaw) from a mutex-protected global struct. This is the part I'm most interested in extending later: right now it's a stub with a hardcoded/simulated reading, but the shape is already there for a real sensor feed to push data in and the server to serve the latest value out.
- Every accepted connection gets handed to the thread pool instead of being handled inline, so one slow client can't block the others.
- A visitor counter behind its own mutex, mostly as a second, simpler example of shared state that needs locking alongside the IMU struct.

## Why raw sockets instead of a library

The point of this project wasn't to serve files fast, it was to actually understand what an HTTP server is doing underneath a framework: reading raw bytes off a socket, figuring out where the request ends, matching a route by string comparison, and writing a properly formatted response back out — status line, headers, blank line, body. Frameworks hide all of that, which is exactly why I didn't want to start with one.

## Concurrency model

Same thread pool design as the `threadpool` project (fixed worker threads, shared job queue, mutex + condition variable). Each accepted connection becomes one job submitted to the pool, so request handling happens off the main accept loop.

## Building

```
mkdir build && cd build
cmake ..
cmake --build .
./jashserver
```

Then visit `http://localhost:<port>/` — the static frontend includes a small 3D aircraft viewer (`aircraft.html`) and the `/imu` demo page (`imu.html`), which is the early version of what I want a real telemetry/flight-data logger to eventually feed.

## What's not done yet

No HTTPS, no chunked transfer encoding, minimal error handling for malformed requests, and the IMU data is currently a stand-in rather than a real sensor feed. Good next step once I get further into the embedded/sensor phase of the roadmap.
