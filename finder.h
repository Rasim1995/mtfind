#ifndef FINDER_H
#define FINDER_H

#include <string>
#include <vector>
#include <map>


using std::string;

class Finder
{
    using ResType = std::map<int, string>;
    using LinesType = std::vector<string>;
public:
    // Если max_thereads = 0 (по умолчанию), оптимальное количество подбирается
    // автоматически.
    Finder(int max_threads = 0);

    // Найти все вхождения mask в текте файла file_path и вернуть их в виде пары
    // {номер строки: вхождение}
    ResType FindAllOccurs(string file_path, string mask);


private:
    ResType all_occurs; // Результат - все вхождения
    LinesType all_lines;
    string mask;

    // Разбить содержимое файла на куски в зависимости от количества заданных
    // потоков и запустить обработку каждого куска в отдельном потоке
    void DistributeTasks();

    // Поиск в рамках одного куска от begin_line до end_line включительно
    void ProcessOneThread(int begin_line, int end_line);

    // Поиск вхождения по маске в одной строке line_number. Основной алгоритм.
    string GetOccurInLine(int line_number);

    // Потокобезопасная запись результата поиска по строке в аккумулирующий
    // словарь all_occurs
    void WriteResult(int line_number, string occur);

};

#endif // FINDER_H
