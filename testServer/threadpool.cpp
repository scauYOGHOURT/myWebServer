#include "threadpool.h"

#include <iostream>

ThreadPool::ThreadPool(int numWorkers) : stop(false)
{
    numWorkers = numWorkers <= 0 ? 1 : numWorkers;
    for (int i = 0; i < numWorkers; ++i)
    {
        threads.emplace_back([this]() {
            while (1)
            {
                Task task;
                {
                    std::unique_lock<std::mutex> lock(TPlock);
                    while (!stop && tasks.empty())
                        cond.wait(lock);

                    if (tasks.empty() && stop)
                    {
                        return;
                    }
                    task = tasks.front();
                    tasks.pop();
                }

                if (task)
                {
                    task();
                }
            }
        });
    }
}

void ThreadPool::pushTask(const Task &task)
{
    {
        std::unique_lock<std::mutex> lock(TPlock);
        tasks.push(task);
    }

    cond.notify_one();
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(TPlock);
        stop = true;
    }

    cond.notify_all();
    for (auto &thread : threads)
        thread.join();
}
