#include "finder.h"
#include "logger.h"
#include "thread_pool.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <regex>
#include <thread>

Finder::Finder(int max_threads) : m_max_threads(max_threads) {}

Finder::ResType Finder::FindAllOccurs(string file_path, string mask)
{
    Logger logger(__FUNCTION__, file_path, mask);

    std::ifstream stream(file_path);

    if(!stream.is_open())
    {
        std::cout << "Could not open the file " << file_path << "!\n";
        return m_all_occurs;
    }

    auto file_size = std::filesystem::file_size(file_path);

    logger << "file size = " << file_size << '\n';

    InitRegexpPattern(mask);
    DistributeTasks(stream, file_size, CalcOptimalPartSize(file_size));

    return m_all_occurs;
}

void Finder::InitRegexpPattern(const string& mask)
{
    Logger logger(__FUNCTION__, mask);

    // Делаем из маски регулярку, экранировав спецсимволы
    std::regex special_symbools(R"([\.\+\*\^\$\(\)\[\]\{\}\|\\])");
    string regex_content = std::regex_replace(mask, special_symbools, "\\$&");
    // и заменив спецсимвол маски '?', обозначающий либой символ, на точку
    std::regex question_mark(R"(\?)");
    regex_content = std::regex_replace(regex_content, question_mark, ".");

    std::cout << "regex contetn = " << regex_content << '\n';

    try
    {
        m_pattern.assign(regex_content);
    } catch (std::exception &ex)
    {
        std::cerr << "ERROR: Failed to generate correct regular expression using mask "
                  << mask << ". Details: " << ex.what() << '\n';
        std::exit(-1);
    }
}

bool Finder::ReadLines(std::ifstream& stream, size_t max_bytes_count, size_t& curr_line_number, LinesType& res_lines_buffer)
{
    Logger logger(__FUNCTION__);

    string buf;
    size_t bytes_counter = 0;

    while(std::getline(stream, buf) && bytes_counter < max_bytes_count)
    {
        res_lines_buffer.push_back({curr_line_number, buf});
        bytes_counter += buf.size();
        ++curr_line_number;
    }

    return bytes_counter >= max_bytes_count; // Есть ли еще
}

size_t Finder::CalcOptimalPartSize(size_t file_size)
{
    Logger logger(__FUNCTION__);

    static constexpr int max_part_size = 1000000;

    // Определяем, сколько потоков доступно
    if(!m_max_threads)
        m_max_threads = std::max(1u, std::thread::hardware_concurrency());

    logger << m_max_threads << " threads available\n";

    // Определяем, по сколько строк распределить каждому потоку
    size_t part_size =
        std::min(static_cast<int>((file_size / m_max_threads)), max_part_size);
    part_size = part_size ? part_size : file_size;

    return part_size; // Байт
}

void Finder::DistributeTasks(std::ifstream& stream, size_t file_size, size_t part_size)
{
    Logger logger(__FUNCTION__);

    ThreadPool thread_pool(m_max_threads);

    // Заполняем рабочий буфер
    for(auto i = 0; i < m_max_threads; ++i)
    {
        m_work_buffers.push_back(LinesType());
        m_free_work_buffers.push(i);
    }

    size_t line_counter = 0;
    bool has_more = true;

    // Последовательно считываем по part_size байт и закидываем на обработку в пул потоков
    while(has_more)
    {
        auto free_buffer_idx = GetFreeWorkBuffer();

        has_more = ReadLines(stream, part_size, line_counter, m_work_buffers[free_buffer_idx]);

        std::function<void()> task = [this, free_buffer_idx]() { ProcessOneThread(free_buffer_idx); };
        thread_pool.AddTask(task);
    }
}

void Finder::ProcessOneThread(size_t buffer_idx)
{
    Logger logger(__FUNCTION__, std::to_string(buffer_idx));

    for(auto& line : m_work_buffers[buffer_idx])
    {
        if( auto occur = ProcessOneLine(line.second))
            WriteResult(line.first, *occur);
    }

    FreeWorkBuffer(buffer_idx);
}

void Finder::FreeWorkBuffer(size_t buffer_index)
{
    std::unique_lock lock(m_work_buffer_mutex);
    Logger logger(__FUNCTION__, std::to_string(buffer_index));
    m_free_work_buffers.push(buffer_index);
    m_work_buffer_state_cond.notify_one();
    m_work_buffers[buffer_index].clear();
}

size_t Finder::GetFreeWorkBuffer()
{
    std::unique_lock lock(m_work_buffer_mutex);
    Logger logger(__FUNCTION__);

    while(true)
    {
        if (m_free_work_buffers.size())
        {
            auto res = m_free_work_buffers.front();
            m_free_work_buffers.pop();
            return res;
        }
        m_work_buffer_state_cond.wait(lock, [this]{ return !m_free_work_buffers.empty(); });
    }
}

std::optional<std::pair<size_t, string>> Finder::ProcessOneLine(const string& line)
{
    Logger logger(__FUNCTION__, line);

    std::optional<std::pair<size_t, string>> res;

    std::smatch occur;
    std::regex_search(line, occur, m_pattern);

    if(occur.size())
        res = {occur.position(), occur.str()};

    return res;
}

void Finder::WriteResult(size_t line_number, std::pair<size_t, string> occur)
{
    Logger logger(__FUNCTION__, std::to_string(line_number), occur.second);

    const std::scoped_lock lock(m_write_result_mutex);

    // std::cout << line_number << " " << occur.first + 1 << occur.second << std::endl;

    m_all_occurs[line_number] = occur;
}
