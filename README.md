# ncurses_dinorunner

A C++ implementation of the Chrome dino game using ncurses

![image](https://github.com/user-attachments/assets/1bffca9c-3115-42cd-b8ce-00a04f1bb904)
![image](https://github.com/user-attachments/assets/eff266c8-0c54-48f1-833b-e16ae0b9f766)

## Prerequisites
- CMake  
- Conan 2  

# initial build - note: mv generators/ -> build/ (idk how to properly define the cmakelists.txt)
conan install .. --output-folder=. --build=missing  

cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release  

cmake --build . --config Release  

# rebuild 
cmake --build . --target clean  
cmake --build . --config Release  
