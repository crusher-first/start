#ifndef TIMER_H
#define TIMER_H

#include <queue>
#include <chrono>
#include <mutex>
#include <functional>

struct TimerNode {
    int fd;
    std::chrono::steady_clock::time_point expire;
    std::function<void()> callback;
    
    bool operator>(const TimerNode& other) const {
        return expire > other.expire;
    }
};

class Timer {
public:
    Timer();
    ~Timer();

    void AddTimer(int fd, int timeout, std::function<void()> callback);
    void DelTimer(int fd);
    void Tick();
    void Pop();
    int GetNextTick();

private:
    std::priority_queue<TimerNode, std::vector<TimerNode>, std::greater<TimerNode>> m_queue;
    std::mutex m_mutex;
};

#endif // TIMER_H
