cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(libtepertokrem)

project(tepertokrem)


set(TEPERTO_DEBUG_OPTIONS -ggdb -O0 -Wall -Wextra -pedantic -march=native -fsanitize=address -fno-omit-frame-pointer -pthread)
set(TEPERTO_RELEASE_OPTIONS -O3 -Wextra -pedantic -march=native -pthread)
set(TEPERTO_DEBUG_LIBRARIES -lasan -ltepertokrem -lmagic)
set(TEPERTO_RELEASE_LIBRARIES -ltepertokrem -lmagic)

set(TEPERTO_DEBUG_LIBDIR "${CMAKE_SOURCE_DIR}/cmake-build-debug/libtepertokrem")
set(TEPERTO_RELEASE_LIBDIR "${CMAKE_SOURCE_DIR}/cmake-build-release/libtepertokrem")

add_executable(tepertokrem
        tepertokrem/main.cpp)
add_dependencies(tepertokrem libtepertokrem)
target_include_directories(tepertokrem PRIVATE libtepertokrem)
target_link_directories(tepertokrem PRIVATE "$<$<CONFIG:DEBUG>:${TEPERTO_DEBUG_LIBDIR}>")
target_link_directories(tepertokrem PRIVATE "$<$<CONFIG:RELEASE>:${TEPERTO_RELEASE_LIBDIR}>")
target_compile_options(libtepertokrem PRIVATE "$<$<CONFIG:DEBUG>:${TEPERTO_DEBUG_OPTIONS}>")
target_compile_options(libtepertokrem PRIVATE "$<$<CONFIG:RELEASE>:${TEPERTO_RELEASE_OPTIONS}>")
target_link_libraries(tepertokrem -lasan -ltepertokrem -lmagic)