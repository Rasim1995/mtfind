cmake_minimum_required(VERSION 3.5)

project(mtfind LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(mtfind
    src/main.cpp
    src/finder.h src/finder.cpp
    src/logger.h
    src/thread_pool.h src/thread_pool.cpp
)



