cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

project(libtepertokrem)

add_library(libtepertokrem
        tepertokrem.hpp
        utility/named_type.hpp types.hpp
        application/application.hpp application/application.cpp
        stream/stream.hpp stream/stream.cpp
        address.hpp address.cpp
        stream/tcpstream.hpp stream/tcpstream.cpp
        stream/stream2.hpp stream/stream2.cpp
        http/http.hpp http/http.cpp
        http/request.hpp http/request.cpp
        )
set_target_properties(libtepertokrem PROPERTIES PREFIX "")
target_compile_options(libtepertokrem PRIVATE -ggdb -O0 -Wall -Wextra -pedantic -march=native -fsanitize=address -fno-omit-frame-pointer)
target_link_libraries(libtepertokrem -lasan)