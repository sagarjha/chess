cmake_minimum_required(VERSION 3.14.0)

include_directories(${chess_SOURCE_DIR})

add_executable(main main.cpp)
target_link_libraries(main chess)
