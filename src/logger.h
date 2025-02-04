#pragma once

#include <string>
#include <sstream>
#include <iostream>

using std::string;

/**
 * @brief Простой помощник для профилирования путем вывода логов в конструкторе и деструкторе
 */
class Logger
{
public:
#ifndef NDEBUG
    Logger(string method_name, string param1 = "", string param2 = "", string param3 = "")
    {
        std::stringstream ss;

        ss << method_name << "( ";

        if (!param1.empty())
            ss << param1;

        if (!param2.empty())
            ss << ", " << param2;

        if (!param3.empty())
            ss << ", " << param3;

        ss << " )";

        text = ss.str();
        std::cout << "[start] " << text << std::endl;
    }

    ~Logger()
    {
        std::cout<< "[stop] " << text << std::endl;
    }

    void Print(string msg) const
    {
        std::cout << msg << std::endl;
    }

    template< class T >
    const Logger& operator << (T msg) const
    {
        std::cout << msg;

        return *this;
    }
private:
    string text;
#else
    Logger(string method_name, string param1 = "", string param2 = "", string param3 = "") {}

    ~Logger() {}

    void Print(string msg){}

    template< class T >
    const Logger& operator << (T msg) const { return *this;}
#endif
};
