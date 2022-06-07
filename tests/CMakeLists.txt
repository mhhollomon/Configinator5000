# COnfiginator5000/tests

cmake_minimum_required(VERSION 3.13)

## first Test ###########################
add_executable (t01-settings)
target_sources(t01-settings PRIVATE "t01-settings.cpp")
target_link_libraries(t01-settings
    PRIVATE doctest Configinator5000)

## first Test ###########################
add_executable (t02-parse)
target_sources(t02-parse PRIVATE "t02-parse.cpp")
target_link_libraries(t02-parse
    PRIVATE doctest Configinator5000)
