#include <iostream>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include <functional>

class ThreadPool
{
private:
    static const int NUM_THREADS = 4;

    pthread_t threads[NUM_THREADS];
    /*this stores the jobs that are submitted to the thread pool. 
    Each job is represented as a std::function<void()>, which allows
    us to store any callable object that takes no arguments and returns void.*/
    std::queue<std::function<void()>>jobs;

    pthread_mutex_t mutex;
    pthread_cond_t condition;

    bool shutdown;

    static void* workerEntry(void* arg)
    {
        ThreadPool* pool = static_cast<ThreadPool*>(arg);
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
                std::cout << "Worker: No jobs, going to wait..." << std::endl;
                pthread_cond_wait(&condition, &mutex);
            }

            if (shutdown && jobs.empty())
            {
                pthread_mutex_unlock(&mutex);
                break;
            }

            std::function<void()> job = jobs.front();
            jobs.pop();

            pthread_mutex_unlock(&mutex);

            std::cout << "Thread " << pthread_self() << ": Processing job " << std::endl;
            job();
        }
    }

public:
    ThreadPool(){
        shutdown=false;
        pthread_mutex_init(&mutex,nullptr);
        pthread_cond_init(&condition,nullptr);
        
        /*this refers to the address of the current thread pool object,
        which is passed as an argument to the workerEntry function. Inside the workerEntry function
        , we cast this pointer back to a ThreadPool pointer and call the workerLoop method on it.*/
        for(int i=0;i<NUM_THREADS;i++){
            pthread_create(&threads[i],nullptr,workerEntry,this);
        }
    }
    void submit(std::function<void()> job){
        pthread_mutex_lock(&mutex);
        if(shutdown){
            pthread_mutex_unlock(&mutex);
            throw std::runtime_error("ThreadPool is shutting down, cannot submit new jobs.");
            return;
        }
        jobs.push(job);
        std::cout<<"Main thread: Added job "<<std::endl;
        pthread_cond_signal(&condition);
        pthread_mutex_unlock(&mutex);
    }
    ~ThreadPool(){
        pthread_mutex_lock(&mutex);
        shutdown=true;
        pthread_cond_broadcast(&condition);
        pthread_mutex_unlock(&mutex);
        for(int i=0;i<NUM_THREADS;i++){
            pthread_join(threads[i],nullptr);
        }
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&condition);
    }

};

void downloadFile(std::string filename)
{
    std::cout << "Downloading " << filename
              << " on thread " << pthread_self()
              << std::endl;

    sleep(2);
}

void calculate(int a, int b)
{
    std::cout << a << " + " << b
              << " = " << a + b
              << " on thread " << pthread_self()
              << std::endl;

    sleep(2);
}

void processImage(std::string filename)
{
    std::cout << "Processing " << filename
              << " on thread " << pthread_self()
              << std::endl;

    sleep(2);
}
int main()
{
    ThreadPool pool;

    std::string filename = "sr71_manual.pdf";

    pool.submit([filename]() {
        downloadFile(filename);
    });

    pool.submit([]() {
        calculate(10, 20);
    });

    pool.submit([]() {
        processImage("blackbird.jpg");
    });

    return 0;
}
/*When I create one threadpool object there will be 4 thread that will get initailzed coz we have 
4 threads and all of em share the same mutex and conditons 
but they will execute workerloop function*/

