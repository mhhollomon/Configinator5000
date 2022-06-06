# COnfiginator5000/tests

cmake_minimum_required(VERSION 3.13)

## first Test ###########################
add_executable (t01-start)
target_sources(t01-start PRIVATE "t01-start.cpp")
target_link_libraries(t01-start
    PRIVATE doctest Configinator5000)
