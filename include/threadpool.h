#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
public:
    explicit ThreadPool(int thread_num = 4);
    ~ThreadPool();

    template<typename T>
    void AddTask(T&& task) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_tasks.emplace(std::forward<T>(task));
        }
        m_cond.notify_one();
    }

    void Stop();

private:
    void Worker() noexcept;

    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::atomic<bool> m_running;
    int m_thread_num;
};

#endif // THREADPOOL_H
