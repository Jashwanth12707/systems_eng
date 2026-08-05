
/*this file is created to demonstrate a race condition by creating two 
threads that increment the same variable by passing out the address of the 
counter variable and instead of dereferencing it we store 
the address directly and dereference it when it is the time to increment  */
#include <iostream>
#include <pthread.h>
struct ThreadData{
    int* counter;
    pthread_mutex_t* mutex;
};

void* increment(void* arg){  
    struct ThreadData* data = static_cast<struct ThreadData*>(arg);
    int* num = data->counter;
    pthread_mutex_t* mutex = data->mutex;
    pthread_mutex_lock(mutex);
    for(int i=0;i<100000;i++){
        
        (*num)++;
        pthread_mutex_unlock(mutex);
    }
    return nullptr;
}
int main(){
    int counter=0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_t thread1,thread2;
    ThreadData data = {&counter, &mutex};
    pthread_create(&thread1,nullptr,increment,&data);
    pthread_create(&thread2,nullptr,increment,&data);
    pthread_join(thread1,nullptr);
    pthread_join(thread2,nullptr);
    std::cout<<"Final counter value: "<<counter<<std::endl;
}

/*| Code                        | What you get                    |
| --------------------------- | ------------------------------- |
| `static_cast<int*>(arg)`    | address (`int*`)                |
| `*static_cast<int*>(arg)`   | integer at that address (`int`) |
| `int* num = ...`            | `num` stores address            |
| `int num = ...`             | `num` stores integer value      |
| `num` when `num` is `int*`  | address                         |
| `*num` when `num` is `int*` | integer being pointed to        |*/
