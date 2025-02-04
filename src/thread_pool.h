#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

/**
 * @brief Примитивный пул потоков с минимально необходимым функционалом.
 */
class ThreadPool
{
    using ThreadsType = std::vector<std::thread>;
    using TaskType = std::function<void()>;
    using QueueType = std::queue<TaskType>;
public:
    // Инициализировать потоки
    ThreadPool(int thread_count);

    // Дождаться завершения всех задач и остановить потоки.
    ~ThreadPool();

    // Добавить новую задачу на выполнение
    void AddTask(TaskType &func);

private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    void Loop();

    ThreadsType             m_threads;
    QueueType               m_tasks;
    std::condition_variable m_condition;
    std::mutex              m_mutex;
    std::atomic<bool>       m_should_terminate = false;
};

