#pragma once

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <regex>
#include <string>
#include <thread>
#include <vector>

using std::string;

/**
 * @brief Реализация потокового поиска подстроки в текстовом файле по маске в многопоточном режиме
 */
class Finder
{
    using ResType = std::map<size_t, std::pair<size_t, string>>;   // { line_number: <occur_position, occur> }
    using LinesType = std::vector<std::pair<size_t, string>>;          // [ <line_number, line_content>, ... ]
    using ThreadsType = std::vector<std::unique_ptr<std::thread>>;  // [ *thread1, ... ]
public:
    // Если max_thereads = 0 (по умолчанию), оптимальное количество подбирается
    // автоматически.
    Finder(int max_threads = 0);

    // Найти все вхождения mask в текте файла file_path и вернуть их в виде пары
    // {номер строки: вхождение}
    ResType FindAllOccurs(string file_path, string mask);

private:
    Finder(const Finder&) = delete;
    Finder& operator=(const Finder&) = delete;
    Finder(Finder&&) = delete;
    Finder& operator=(Finder&&) = delete;

    // Преобразовать маску для поиска вида "some?mask." в регулярное выражение вида "some.mask\."
    void InitRegexpPattern(const string& mask);

    // Построчно читать поток stream кусками по max_bytes_count байтов (округляя
    // до целой строки, т.е. фактически может вернуть больше). В curr_line_number
    // записать текущей номер прочитанной строки. Результирующие строки сложить
    // в res_lines_buffer.
    bool ReadLines(std::ifstream &stream, size_t max_bytes_count,
                   size_t &curr_line_number, LinesType &res_lines_buffer);

    // Расчитать по размеру файла и доступному количеству потоков оптимальный
    // кусок для обработки одним потоком. Чем больше файл, тем больше кусок, но
    // с ограничением до условного максимума, чтобы на больших файлах не
    // отъедать много памяти
    size_t CalcOptimalPartSize(size_t file_size);

    // Распределить разбитые куски по потокам
    void DistributeTasks(std::ifstream &stream, size_t file_size, size_t part_size);

    // Поиск в рамках одного куска
    void ProcessOneThread(size_t buffer_idx);

    // Освободить рабочий буфер с индексом buffer_index. Рабочий буфер -
    // выделенная область памяти для оперерования одним потоком
    void FreeWorkBuffer(size_t buffer_index);
    // Получить свободный буфер из пула
    size_t GetFreeWorkBuffer();

    // Поиск вхождения по маске в одной строке line_number. Основной алгоритм.
    std::optional<std::pair<size_t, string>> ProcessOneLine(const string& line);

    // Потокобезопасная запись результата поиска по строке в аккумулирующий
    // словарь all_occurs
    void WriteResult(size_t line_number, std::pair<size_t, string> occur);

    ResType                  m_all_occurs; // Результат - все вхождения
    std::regex               m_pattern;    // Маска преобразованная в регулярку
    int                      m_max_threads = 0;    // Кол-во достпуных потоков в системе, либо задается снаружи.
    std::vector<LinesType>   m_work_buffers;       // Пул рабочих буферов отдельно для кажд. потока
    std::queue<size_t>       m_free_work_buffers;  // Очередь доступных рабочих буферов
    std::mutex               m_write_result_mutex; // Для синхронизации результата в словарь
    std::mutex               m_work_buffer_mutex;  // Для синхронизации очереди рабочего буфера
    std::condition_variable  m_work_buffer_state_cond; // Будит поток, ожидающий освобождения рабочего буфера
};
