#include "threadpool.h"

ThreadPool::ThreadPool(int thread_num) 
    : m_running(true), m_thread_num(thread_num) {
    for (int i = 0; i < m_thread_num; ++i) {
        m_threads.emplace_back(&ThreadPool::Worker, this);
    }
}

ThreadPool::~ThreadPool() {
    Stop();
}

void ThreadPool::Stop() {
    m_running = false;
    m_cond.notify_all();
    for (auto& t : m_threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void ThreadPool::Worker() {
    while (m_running) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock, [this] { return !m_running || !m_tasks.empty(); });
            
            if (!m_running && m_tasks.empty()) return;
            
            task = std::move(m_tasks.front());
            m_tasks.pop();
        }
        if (task) task();
    }
}
