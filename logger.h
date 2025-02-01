#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <sstream>
#include <iostream>

using std::string;

// Простой помощник для профилирования путем вывода логов в конструкторе и деструкторе
class Logger
{
public:
#ifndef NDEBUG
    Logger(string method_name, string param1 = "", string param2 = "", string param3 = "")
    {
        std::stringstream ss;

        ss << method_name << "( ";

        if( !param1.empty() )
            ss << param1;

        if( !param2.empty() )
            ss<< ", " << param2;

        if( !param3.empty() )
            ss<< ", " << param3;

        ss << " )";

        text = ss.str();
        std::cout << "[start] " << text;
    }

    ~Logger()
    {
        std::cout<< "[stop] " << text;
    }

    void Print(string msg)
    {
        std::cout << msg << std::endl;
    }
private:
    string text;
#elif
    Logger(string method_name, string param1 = "", string param2 = "", string param3 = "") {}

    ~Logger() {}

    void Print(string msg){}
#endif
};

#endif // LOGGER_H
