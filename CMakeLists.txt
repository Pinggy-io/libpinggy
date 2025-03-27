
cmake_minimum_required(VERSION 3.15)

# set the project name and version
project(Pinggy VERSION 3.0.19)


# Specify the destination directory (base directory)
if(NOT PINGGY_RELEASE_DIR)
    set(PINGGY_RELEASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

add_subdirectory(src)
