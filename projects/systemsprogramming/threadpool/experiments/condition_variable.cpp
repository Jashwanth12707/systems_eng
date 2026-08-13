#include <iostream>
#include <pthread.h>
#include <unistd.h>

struct SharedData
{
    bool jobAvailable;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
};

void* worker(void* arg)
{
    SharedData* data = static_cast<SharedData*>(arg);
    //brfore checking shared state,acquire its mutex
    pthread_mutex_lock(&data->mutex);
    //no job?wait until someone tells us the state may have changed
    while(!data->jobAvailable)
    {
        std::cout<<"Worker:no job,going to wait..."<<std::endl;
        pthread_cond_wait(&data->condition,&data->mutex);
       
    }
    std::cout<<"Worker:found a job!"<<std::endl;
    pthread_mutex_unlock(&data->mutex);
    return nullptr;
}

int main()
{
    SharedData data;
    data.jobAvailable=false;
    pthread_mutex_init(&data.mutex,nullptr);
    pthread_cond_init(&data.condition,nullptr);
    pthread_t thread;
    pthread_create(&thread,nullptr,worker,&data);
    //give worker time to reach cond_wait
    sleep(2);
    //change the shared state 
    pthread_mutex_lock(&data.mutex);
    data.jobAvailable=true;

    //Notify one worker waiting on this condition variable
    pthread_cond_signal(&data.condition);

    pthread_mutex_unlock(&data.mutex);
    //wait for worker to finish 
    pthread_join(thread,nullptr);

    pthread_mutex_destroy(&data.mutex);
    pthread_cond_destroy(&data.condition);

    return 0;
}