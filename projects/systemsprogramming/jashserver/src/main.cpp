#include <iostream>
#include <queue>
#include <functional>
#include <string>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iterator>
#include <pthread.h>
#include <unistd.h>
#include <unordered_map>
#include <cstdlib>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>

#include "http_request.h"
int visitor_count=0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

struct IMUData
{
    float roll;
    float pitch;
    float yaw;
};

IMUData latest_imu{0.0f, 0.0f, 0.0f};

pthread_mutex_t imu_mutex = PTHREAD_MUTEX_INITIALIZER;

class ThreadPool
{
private:

    static const int NUM_THREADS = 4;

    pthread_t threads[NUM_THREADS];

    // Queue of tasks.

    std::queue<std::function<void()>> jobs;

    pthread_mutex_t mutex;

    pthread_cond_t condition;

    bool shutdown;

    static void* workerEntry(void* arg)
    {
        ThreadPool* pool =
            static_cast<ThreadPool*>(arg);

        pool->workerLoop();

        return nullptr;
    }

    void workerLoop()
    {
        while (true)
        {
            pthread_mutex_lock(&mutex);

            while (jobs.empty() && !shutdown)
            {
                pthread_cond_wait(
                    &condition,
                    &mutex
                );
            }

            if (shutdown && jobs.empty())
            {
                pthread_mutex_unlock(&mutex);

                break;
            }

            auto job = jobs.front();

            jobs.pop();

            pthread_mutex_unlock(&mutex);

            job();
        }
    }

public:

    ThreadPool()
    {
        shutdown = false;

        pthread_mutex_init(&mutex, nullptr);

        pthread_cond_init(&condition, nullptr);

        for (int i = 0; i < NUM_THREADS; i++)
        {
            pthread_create(
                &threads[i],
                nullptr,
                workerEntry,
                this
            );
        }
    }

    void submit(std::function<void()> job)
    {
        pthread_mutex_lock(&mutex);

        jobs.push(job);

        pthread_cond_signal(&condition);

        pthread_mutex_unlock(&mutex);
    }

    ~ThreadPool()
    {
        pthread_mutex_lock(&mutex);

        shutdown = true;

        pthread_cond_broadcast(&condition);

        pthread_mutex_unlock(&mutex);

        for (int i = 0; i < NUM_THREADS; i++)
        {
            pthread_join(
                threads[i],
                nullptr
            );
        }

        pthread_mutex_destroy(&mutex);

        pthread_cond_destroy(&condition);
    }
};
std::string getRoute(const std::string& path)
{
    size_t pos = path.find('?');

    if (pos == std::string::npos)
    {
        return path;
    }

    return path.substr(0, pos);
}

std::unordered_map<std::string, std::string>
parseQuery(const std::string& path)
{
    std::unordered_map<std::string, std::string> params;

    size_t pos = path.find('?');

    if (pos == std::string::npos)
    {
        return params;
    }

    std::string query = path.substr(pos + 1);

    std::stringstream ss(query);

    std::string token;

    while (std::getline(ss, token, '&'))
    {
        size_t equal = token.find('=');
        if (equal == std::string::npos)
        {
            continue;
        }

        params[token.substr(0, equal)] =
            token.substr(equal + 1);
    }

    return params;
}

std::string profileService(
    std::unordered_map<std::string, std::string>& p)
{
    std::string name = p["name"];
    std::string age = p["age"];
    std::string sleep = p["sleep"];
    std::string lang = p["lang"];

    return
        "=== DOSSIER ===\n"
        "Name: " + name + 
        "\nAge: " + age +
        "\nSleep: " + sleep +
        " hours\nLanguage: " + lang;
}

std::string judgeService(
    std::unordered_map<std::string, std::string>& p)
{
    std::string name = p["name"];

    int bugs = std::stoi(p["bugs"]);

    if (bugs < 10)
    {
        return name +
               ": acceptable chaos.";
    }

    if (bugs < 50)
    {
        return name +
               ": repository requires emergency maintenance.";
    }

    return name +
           ": catastrophic failure detected.";
}

long long fibonacci(int n)
{
    if (n <= 1)
    {
        return n;
    }

    return fibonacci(n - 1) +
           fibonacci(n - 2);
}

std::string imuService()
{
    pthread_mutex_lock(&imu_mutex);

    float roll = latest_imu.roll;
    float pitch = latest_imu.pitch;
    float yaw = latest_imu.yaw;

    pthread_mutex_unlock(&imu_mutex);

    return
        "{"
        "\"roll\":" + std::to_string(roll) +
        ",\"pitch\":" + std::to_string(pitch) +
        ",\"yaw\":" + std::to_string(yaw) +
        "}";
}

std::string fibonacciService(
    std::unordered_map<std::string, std::string>& p)
{
    int n = std::stoi(p["n"]);

    return "Fib(" +
           std::to_string(n) +
           ") = " +
           std::to_string(fibonacci(n));
}

// ----------------------------------------------------------
// This function contains your old server logic.
// The only difference:
//
// Instead of running in main(),
// it runs inside a worker thread.
// ----------------------------------------------------------

std::string homeService()
{
    return "Hello from JashServer!";
}

std::string timeService()
{
    auto now =
        std::chrono::system_clock::now();

    std::time_t current_time =
        std::chrono::system_clock::to_time_t(now);

    return std::ctime(&current_time);
}

std::string sr71Service()
{
    return
        "Aircraft: Lockheed SR-71 Blackbird\n"
        "Top speed: Mach 3.3\n"
        "Service ceiling: 85000 ft\n";
}

std::string counterService()
{
    pthread_mutex_lock(&counter_mutex);

    visitor_count++;

    int current =
        visitor_count;

    pthread_mutex_unlock(&counter_mutex);

    return
        "Visitor count: "
        + std::to_string(current);
}

std::unordered_map<std::string, std::function<std::string()>> routes={
    
    {"/", homeService},
    {"/time", timeService},
    {"/sr71", sr71Service},
    {"/counter", counterService}
};
std::string readFile(const std::string& filename)
    {
    std::ifstream file(filename);
    if(!file){
        return "";
    }
    return std::string(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    }   

std::string getContentType(const std::string& path)
{
    if (path.size() >= 5 &&
        path.substr(path.size() - 5) == ".html")
    {
        return "text/html";
    }

    if (path.size() >= 4 &&
        path.substr(path.size() - 4) == ".css")
    {
        return "text/css";
    }

    return "text/plain";
}


    
void handleClient(
    int client_fd,
    sockaddr_in client_address)
{
    std::cout
        << "\nWorker "
        << pthread_self()
        << " handling "
        << inet_ntoa(client_address.sin_addr)
        << ":"
        << ntohs(client_address.sin_port)
        << std::endl;


    // --------------------------------------------------
    // Receive HTTP headers first
    // --------------------------------------------------

    std::string raw_request;

    char buffer[4096];

    const std::string header_end = "\r\n\r\n";

    while (raw_request.find(header_end) == std::string::npos)
    {
        ssize_t bytes_received =
            recv(
                client_fd,
                buffer,
                sizeof(buffer),
                0
            );

        if (bytes_received <= 0)
        {
            close(client_fd);
            return;
        }

        raw_request.append(
            buffer,
            bytes_received
        );

        // Prevent an absurdly large header
        if (raw_request.size() > 16384)
        {
            close(client_fd);
            return;
        }
    }


    // --------------------------------------------------
    // Parse headers so we can find Content-Length
    // --------------------------------------------------

    HTTPRequest request;

    request.parse(raw_request);

    size_t body_start =
        raw_request.find(header_end) + header_end.length();

    size_t content_length = 0;

    auto content_length_header =
        request.headers.find("Content-Length");

    if (content_length_header != request.headers.end())
    {
        content_length =
            std::stoul(content_length_header->second);
    }


    // --------------------------------------------------
    // Receive the rest of the body
    // --------------------------------------------------

    while (
        raw_request.size() - body_start
        < content_length
    )
    {
        ssize_t bytes_received =
            recv(
                client_fd,
                buffer,
                sizeof(buffer),
                0
            );

        if (bytes_received <= 0)
        {
            close(client_fd);
            return;
        }

        raw_request.append(
            buffer,
            bytes_received
        );
    }


    // --------------------------------------------------
    // Now parse the COMPLETE HTTP request
    // --------------------------------------------------

    request.parse(raw_request);


    std::cout
        << request.method
        << " "
        << request.path
        << std::endl;


    std::string route =
        getRoute(request.path);

    auto params =
        parseQuery(request.path);


    std::string body;

    std::string status =
        "HTTP/1.1 200 OK\r\n";


    // --------------------------------------------------
    // IMU POST
    // --------------------------------------------------

    if (
        route == "/imu" &&
        request.method == "POST"
    )
    {
        std::cout
            << "\n=== IMU DATA ===\n";

        std::cout
            << request.body
            << std::endl;


        float roll = 0.0f;
        float pitch = 0.0f;
        float yaw = 0.0f;


        int result =
            sscanf(
                request.body.c_str(),
                "{\"roll\":%f,\"pitch\":%f,\"yaw\":%f}",
                &roll,
                &pitch,
                &yaw
            );


        if (result == 3)
        {
            pthread_mutex_lock(&imu_mutex);

            latest_imu.roll = roll;
            latest_imu.pitch = pitch;
            latest_imu.yaw = yaw;

            pthread_mutex_unlock(&imu_mutex);


            std::cout
                << "Roll:  "
                << roll
                << std::endl;

            std::cout
                << "Pitch: "
                << pitch
                << std::endl;

            std::cout
                << "Yaw:   "
                << yaw
                << std::endl;


            body =
                "IMU data received";
        }
        else
        {
            std::cout
                << "Invalid IMU JSON"
                << std::endl;

            status =
                "HTTP/1.1 400 Bad Request\r\n";

            body =
                "Invalid IMU data";
        }
    }


    // --------------------------------------------------
    // IMU GET
    // --------------------------------------------------

    else if (
        route == "/imu" &&
        request.method == "GET"
    )
    {
        body =
            imuService();
    }


    // --------------------------------------------------
    // Existing routes
    // --------------------------------------------------

    else if (route == "/profile")
    {
        body =
            profileService(params);
    }

    else if (route == "/judge")
    {
        body =
            judgeService(params);
    }

    else if (route == "/fibonacci")
    {
        body =
            fibonacciService(params);
    }

    else
    {
        std::string filename =
            "public" + route;

        body =
            readFile(filename);


        if (body.empty())
        {
            auto it =
                routes.find(route);

            if (it != routes.end())
            {
                body =
                    it->second();
            }
            else
            {
                status =
                    "HTTP/1.1 404 Not Found\r\n";

                body =
                    "404 Not Found";
            }
        }
    }


    // --------------------------------------------------
    // HTTP response
    // --------------------------------------------------

    std::string content_type =
        getContentType(route);


    std::string response =
        status +
        "Content-Type: "
        + content_type
        + "\r\n"
        + "Content-Length: "
        + std::to_string(body.length())
        + "\r\n\r\n"
        + body;


    send(
        client_fd,
        response.c_str(),
        response.length(),
        0
    );


    close(client_fd);
}
// ----------------------------------------------------------
// Main server.
// ----------------------------------------------------------

int main()
{
    ThreadPool pool;

    int server_fd =
        socket(
            AF_INET,
            SOCK_STREAM,
            0
        );

    sockaddr_in address{};

    address.sin_family = AF_INET;

    address.sin_addr.s_addr = INADDR_ANY;

    address.sin_port = htons(8080);

    bind(
        server_fd,
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address)
    );

    listen(server_fd, 10);

    std::cout
        << "Server listening on 8080\n";

    while (true)
    {
        sockaddr_in client_address{};

        socklen_t client_length =
            sizeof(client_address);

        int client_fd =
            accept(
                server_fd,
                reinterpret_cast<sockaddr*>(
                    &client_address
                ),
                &client_length
            );

        if (client_fd == -1)
        {
            continue;
        }

        std::cout
            << "Accepted connection\n";

        // Instead of handling the client here,
        // submit the work to the thread pool.

        pool.submit(
            [client_fd, client_address]()
            {
                handleClient(
                    client_fd,
                    client_address
                );
            }
        );
    }

    close(server_fd);

    return 0;
}