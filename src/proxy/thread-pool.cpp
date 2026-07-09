#include "../../include/thread-pool.hpp"


//Constructor for threadpool
ThreadPool::ThreadPool(size_t num_threads){
    //generate num_thread threads to call worker loop, ie handle clients when requests come in
    for (size_t i = 0; i < num_threads; i++){
        //emplace back constructs thread inside of worker vector
        //workers.push_back(std::thread([this]() {worker_loop();}))
        workers.emplace_back([this](){
            worker_loop();
        });
    }
}


//destructor for threadpool
ThreadPool::~ThreadPool(){
    {
        //lock queue to ensure no more enq's
        //stop condition for threads to stop handling clients
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop = true;
    }

    //wake all workers 
    //call workers to join once threads task is completed
    condition.notify_all();
    for (std::thread& worker : workers){
        if (worker.joinable()){
            worker.join();
        }
    }
}
void ThreadPool::enqueue(std::function<void()> task){
    {
        //lock task queue to ensure thread safety
        std::lock_guard<std::mutex> lock(queue_mutex);
        tasks.push(task);
    }
    condition.notify_one();
}

void ThreadPool::worker_loop(){
    while (true){
        std::function<void()> task;
        {
            //worker locks queue
            std::unique_lock<std::mutex> lock(queue_mutex);

            //worker waits until either stop or task is added to queue
            //[this] allows condition.wait to access stop and tasks
            condition.wait(lock, [this](){
                return stop || !tasks.empty();
            });

            if (stop && tasks.empty()){
                return;
            }
            //pop task from queue of tasks
            task = tasks.front();
            tasks.pop();
        }
        //call task
        task();
    }

}
