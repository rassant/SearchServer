cmake_minimum_required(VERSION 3.10)
project(search-server)
set(CMAKE_CXX_STANDARD 17)
find_package(TBB REQUIRED)
set(CMAKE_CXX_COMPILER clang++)

set(SOURCE_FILES
    source/core/document.cpp
    source/core/process_queries.cpp
    source/core/request_queue.cpp
    source/core/search_server.cpp
    source/core/string_processing.cpp

    source/input_output/read_input_functions.cpp

    source/tests/test_example_functions.cpp
    source/tests/test_speed.cpp

    source/utils/data_generator.cpp

    main.cpp
    example.cpp
)
set (PROGRAMM_NAME output)

add_executable(output ${SOURCE_FILES})

target_link_libraries(${PROGRAMM_NAME} ${TBB_LIBRARIES} TBB::tbb)

target_compile_options(${PROGRAMM_NAME} PUBLIC -ggdb -O3)
target_compile_options(${PROGRAMM_NAME} PUBLIC -Wall -Wextra)
