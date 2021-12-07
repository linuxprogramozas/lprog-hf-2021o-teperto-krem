cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

project(libtepertokrem)

set(TEPERTO_DEBUG_OPTIONS -ggdb -O0 -Wall -Wextra -pedantic -march=native -fsanitize=address -fno-omit-frame-pointer -pthread)
set(TEPERTO_RELEASE_OPTIONS -O3 -Wextra -pedantic -march=native -pthread)
set(TEPERTO_DEBUG_LIBRARIES -lasan -lmagic)
set(TEPERTO_RELEASE_LIBRARIES -lmagic)

add_library(libtepertokrem
        tepertokrem.hpp
        utility/named_type.hpp types.hpp
        application/application.hpp application/application.cpp
        application/router.hpp application/router.cpp
        address.hpp address.cpp
        stream/stream2.hpp stream/stream2.cpp
        http/http.hpp http/http.cpp
        http/request.hpp http/request.cpp
        http/handle.hpp http/handle.cpp
        http/responsewriter.hpp http/responsewriter.cpp
        http/status.hpp
        http/header.hpp http/header.cpp
        utility/fileloader.hpp utility/fileloader.cpp
        http/fileserver.hpp http/fileserver.cpp
        utility/url.hpp utility/url.cpp
        )
set_target_properties(libtepertokrem PROPERTIES PREFIX "")
target_compile_options(libtepertokrem PRIVATE "$<$<CONFIG:DEBUG>:${TEPERTO_DEBUG_OPTIONS}>")
target_compile_options(libtepertokrem PRIVATE "$<$<CONFIG:RELEASE>:${TEPERTO_RELEASE_OPTIONS}>")
target_link_libraries(libtepertokrem -lasan -lmagic)