cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(libtepertokrem)

project(tepertokrem)
add_executable(tepertokrem
        tepertokrem/main.cpp)
add_dependencies(tepertokrem libtepertokrem)
target_include_directories(tepertokrem PRIVATE libtepertokrem)
target_link_directories(tepertokrem PRIVATE cmake-build-debug/libtepertokrem)
target_compile_options(tepertokrem PRIVATE -ggdb -O0 -Wall -Wextra -pedantic -march=native -fsanitize=address -fno-omit-frame-pointer)
target_link_libraries(tepertokrem -lasan -ltepertokrem)