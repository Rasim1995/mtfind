#include "thread_pool.h"

ThreadPool::ThreadPool(int thread_count)
{
    for(auto i = 0; i < thread_count; ++i)
    {
        m_threads.emplace_back(std::thread(&ThreadPool::Loop, this));
    }
}

ThreadPool::~ThreadPool()
{
    m_should_terminate = true;
    m_condition.notify_all();

    for(auto& thread : m_threads)
        thread.join();

    m_threads.clear();
}

void ThreadPool::Loop()
{
    while(true)
    {
        TaskType task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this]{ return !m_tasks.empty() || m_should_terminate; });

            if(m_tasks.empty() && m_should_terminate)
                return;

            task = m_tasks.front();
            m_tasks.pop();
        }

        task();
    }
}

void ThreadPool::AddTask(TaskType& func)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    m_tasks.push(func);

    m_condition.notify_one();
}
