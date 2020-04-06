#include "timer.h"
#include "request.h"
void TimerManager::addTimer(Request *request, const int &timeout, const CallBack &cb)
{
    std::lock_guard<std::mutex> lock(TMlock);

    updateTime();

    Timer *timer = new Timer(now + (MS)timeout, cb);
    timerQueue.push(timer);

    if (request->getTimer() != nullptr)
        delTimer(request);

    request->setTimer(timer);
}

void TimerManager::delTimer(Request *request)
{
    Timer *timer = request->getTimer();

    if (timer == nullptr)
        return;

    timer->del();
    request->setTimer(nullptr);
}

void TimerManager::handleExpireTimers()
{
    std::lock_guard<std::mutex> lock(TMlock);

    updateTime();

    while (!timerQueue.empty())
    {
        Timer *timer = timerQueue.top();

        if (timer->isdeleted())
        {
            timerQueue.pop();

            delete timer;
            continue;
        }

        if (std::chrono::duration_cast<MS>(timer->getExpireTime() - now).count() > 0)
        {
            return;
        }

        timer->runCallBack();
        timerQueue.pop();

        delete timer;
    }
}

int TimerManager::getNextExpireTimer()
{
    std::lock_guard<std::mutex> lock(TMlock);

    int res = -1;
    while (!timerQueue.empty())
    {
        Timer *timer = timerQueue.top();
        if (timer->isdeleted())
        {
            timerQueue.pop();
            delete timer;
            continue;
        }
        res = std::chrono::duration_cast<MS>(timer->getExpireTime() - now).count();
        res = (res < 0 ? 0 : res);
        return res;
    }
}