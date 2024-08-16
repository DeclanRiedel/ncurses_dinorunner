cmake_minimum_required(VERSION 3.15)
project(ncurses_dinorunner)

set(CMAKE_CXX_STANDARD 17)

find_package(Curses REQUIRED)

add_executable(dinorunner src/main.cpp)
target_link_libraries(dinorunner PRIVATE Curses::Curses)