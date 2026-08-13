#include <iostream>
#include <pthread.h>

void* threadFunction(void* arg)
{
    int num = *static_cast<int*>(arg);
    std::cout << "Hello from thread! Value: " << num << std::endl;
    return nullptr;
}

int main(){
    pthread_t thread;//Give me a variable called thread capable of holding the pthread identifier that the pthread library uses for a thread.
    int num=42;
    pthread_create(&thread,nullptr,threadFunction,&num);
    
    std::cout<<"hello from main thread!"<<std::endl;
    pthread_join(thread, nullptr);
}


