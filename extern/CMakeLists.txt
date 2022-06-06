
# Configinator5000/etern

cmake_minimum_required(VERSION 3.18)

include(FetchContent)


FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.8
    )

FetchContent_MakeAvailable( doctest )
