cmake_minimum_required(VERSION 3.5)

project(mtfind LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(mtfind main.cpp
    finder.h finder.cpp logger.h
    thread_pool.h thread_pool.cpp
)

include(GNUInstallDirs)
install(TARGETS mtfind
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)



