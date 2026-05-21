#include "timer.h"
#include <unordered_map>
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
    node.valid = true;
    m_queue.push(node);
    m_fd_map[fd] = node;
}

void Timer::DelTimer(int fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_fd_map[fd] = TimerNode();  // Mark as invalid
    m_fd_map.erase(fd);
}

void Timer::Tick() {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto now = std::chrono::steady_clock::now();
    
    while (!m_queue.empty()) {
        TimerNode node = m_queue.top();
        if (node.expire > now) break;
        
        m_queue.pop();
        
        // Only fire if this timer is still valid
        auto it = m_fd_map.find(node.fd);
        if (it != m_fd_map.end() && it->second.expire == node.expire) {
            if (node.callback) {
                node.callback();
            }
            m_fd_map.erase(node.fd);
        }
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
