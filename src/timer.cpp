#include "timer.h"
#include <unistd.h>

Timer::Timer() {
}

Timer::~Timer() {
}

void Timer::AddTimer(int fd, int timeout, std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    TimerNode node;
    node.fd = fd;
    node.expire = std::chrono::steady_clock::now() + std::chrono::seconds(timeout);
    node.callback = callback;
    m_queue.push(node);
}

void Timer::DelTimer(int fd) {
    // 简化实现：实际应该标记删除或使用 unordered_map 索引
}

void Timer::Tick() {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto now = std::chrono::steady_clock::now();
    
    while (!m_queue.empty()) {
        TimerNode node = m_queue.top();
        if (node.expire > now) break;
        
        if (node.callback) {
            node.callback();
        }
        m_queue.pop();
    }
}

void Timer::Pop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_queue.empty()) {
        m_queue.pop();
    }
}

int Timer::GetNextTick() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) return 30;
    
    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(
        m_queue.top().expire - now).count();
    return diff > 0 ? diff : 0;
}
