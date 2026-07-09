#pragma once
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>

class ThreadPool{
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();
    void enqueue(std::function<void()> task);

private:
    //vector storing threads handling clients
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    //queue mutex to lock queue
    std::mutex queue_mutex;
    bool stop = false;
    std::condition_variable condition;

    void worker_loop();
};