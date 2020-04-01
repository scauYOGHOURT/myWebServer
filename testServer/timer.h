#ifndef __TIMER_H__
#define __TIMER_H__

#include <functional>
#include <chrono>
#include <queue>
#include <vector>
#include <iostream>
#include <cassert>
#include <mutex>

using CallBack = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using Timestamp = Clock::time_point;
using MS = std::chrono::microseconds;

class Request;

class Timer
{
public:
    Timer(const Timestamp &when, const CallBack &cb) : expireTime(when),
                                                       callback(cb),
                                                       deleted(false)
    {
    }
    void del() { deleted = true; }
    bool isdeleted() { return deleted; }
    Timestamp getExpireTime() { return expireTime; }
    void runCallBack() { callback(); }
    bool operator<(Timer *x) const
    {
        return expireTime > x->getExpireTime();
    }
    ~Timer() {}

private:
    Timestamp expireTime;
    CallBack callback;
    bool deleted;
};

class TimerManager
{
public:
    TimerManager(Timestamp now) : now(now) {}

    void updateTime() { now = Clock::now(); }
    void addTimer(Request *requert, const int &timeout, const CallBack &cb) {}
    void delTimer(Request *requert) {}
    void handleExpireTime() {}
    int getExpireTime() {}

    ~TimerManager() {}

private:
    using TimerQueue = std::priority_queue<Timer *>;

    TimerQueue timerQueue;
    Timestamp now;
    std::mutex TMlock;
};

#endif