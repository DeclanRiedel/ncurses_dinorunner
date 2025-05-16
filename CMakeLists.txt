cmake_minimum_required(VERSION 3.15)
project(ncurses_dinorunner)
set(CMAKE_CXX_STANDARD 17) # cpp standard

# conan2
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

# executable
add_executable(dinorunner src/main.cpp)

# link conan libs
find_package(Curses REQUIRED)
target_link_libraries(dinorunner PRIVATE Curses::Curses)
