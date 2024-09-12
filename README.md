# ncurses_dinorunner

A C++ implementation of the Chrome dino game using ncurses

## Prerequisites
- C++ compiler
- CMake
- Conan 2

# initial build - mv generators to build dir (idk how to properly define the cmakelists.txt)
conan install .. --output-folder=. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# rebuild 
cmake --build . --target clean
cmake --build . --config Release

# windows
export PATH=$PATH:"/c/Program Files/CMake/bin"
   conan install . --output-folder=build
      cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
         cmake --build build --config Release

builder shit defined inside conanfile



# futureideas: 

use anssi code blocks (not original ascii i dont think) & zoom out terminal for high res? 

   "     ███████",
    "   █████ ████",
    "  ██████████",
    " ████████    ",
     ██████████ ",
   █████████  ██   ",
 ███ "█████        ",
██   " ██ ██       ",
█    " ██ ██       ",
