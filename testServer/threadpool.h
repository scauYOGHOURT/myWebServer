#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool
{
public:
    using Task = std::function<void()>;

    ThreadPool(int numWorkers);

    void pushTask(const Task &task);

    ~ThreadPool();

private:
    std::vector<std::thread> threads;
    std::mutex TPlock;
    std::condition_variable cond;
    std::queue<Task> tasks;
    bool stop;
};

#endif