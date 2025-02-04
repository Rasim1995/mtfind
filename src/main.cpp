#include "finder.h"
#include <iostream>
// #include <chrono>

using namespace std;

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::cout << "It is necessary to pass all parameters: file path and mask!\n";
        return -1;
    }

    string file_path = argv[1];
    string mask = argv[2];

    // auto begin = std::chrono::steady_clock::now();
    auto occurs = Finder().FindAllOccurs(file_path, mask);
    // auto end = std::chrono::steady_clock::now();

    std::cout << occurs.size() << std::endl;

    for( const auto& occur : occurs )
    {
        std::cout << occur.first + 1 << " "
                  << occur.second.first + 1 << " "
                  << occur.second.second
                  << " " << std::endl;
    }

    // std::cout << "Ellapsed time = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[microsecons]" << std::endl;
    return 0;
}
