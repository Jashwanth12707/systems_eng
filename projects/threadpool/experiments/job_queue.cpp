#include <queue>
#include <pthread.h>
#include <iostream>
#include <unistd.h>

struct SharedData
{
    std::queue<int> jobs;

    pthread_mutex_t mutex;
    pthread_cond_t condition;

    bool shutdown;
};

void*worker(void*arg){

    SharedData* data=static_cast<SharedData*>(arg);
    while(true){
        pthread_mutex_lock(&data->mutex);
        while(data->jobs.empty() && !data->shutdown){
            std::cout<<"Worker:No jobs,going to wait..."<<std::endl;
            pthread_cond_wait(&data->condition,&data->mutex);
        }
        if(data->shutdown && data->jobs.empty()){
            pthread_mutex_unlock(&data->mutex);
            break;
        }
        int job=data->jobs.front();
        data->jobs.pop();
        pthread_mutex_unlock(&data->mutex);
        std::cout << "Thread "
        << pthread_self()
        << ": Processing job "
        << job
        << std::endl;
        
        sleep(1);
    }
    return nullptr;
}
/*create struct SharedData with a queue of jobs, a mutex, a condition variable, and a shutdown flag. Implement a worker 
function that waits for jobs to be added to the queue and processes them. In the main function, create a SharedData instance,
initialize the mutex and condition variable, create a worker thread, add jobs to the queue, and signal the worker. Finally, 
set the shutdown flag and signal the worker to exit before joining the thread and cleaning up resources.*/
int main(){
    SharedData data;
    data.shutdown=false;
    pthread_mutex_init(&data.mutex,nullptr);
    pthread_cond_init(&data.condition,nullptr);
    const int num_threads=4;
    pthread_t threads[num_threads];
    for(int i=0;i<num_threads;i++){
        pthread_create(&threads[i],nullptr,worker,&data);
    }

    for(int i=1;i<=12;i++){
        pthread_mutex_lock(&data.mutex);
        data.jobs.push(i);
        std::cout<<"Main thread:Added job "<<i<<std::endl;
        pthread_cond_signal(&data.condition);
        pthread_mutex_unlock(&data.mutex);
    }

    pthread_mutex_lock(&data.mutex);
    data.shutdown=true;
    pthread_cond_broadcast(&data.condition);
    pthread_mutex_unlock(&data.mutex);

    for(int i=0;i<num_threads;i++){
        pthread_join(threads[i],nullptr);
    }
    pthread_mutex_destroy(&data.mutex);
    pthread_cond_destroy(&data.condition);

    return 0;
}
