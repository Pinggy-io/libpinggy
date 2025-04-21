
cmake_minimum_required(VERSION 3.15)


find_package(Git REQUIRED)

#==================================
# Get full version string: tag + commits since + short hash
execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Try to parse semantic version if possible
string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" VERSION_MATCH ${GIT_DESCRIBE})
set(PROJECT_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(PROJECT_VERSION_MINOR "${CMAKE_MATCH_2}")
set(PROJECT_VERSION_PATCH "${CMAKE_MATCH_3}")

# Assign version components or default to 0
if(CMAKE_MATCH_COUNT GREATER 2)
    set(PROJECT_VERSION_MAJOR "${CMAKE_MATCH_1}")
    set(PROJECT_VERSION_MINOR "${CMAKE_MATCH_2}")
    set(PROJECT_VERSION_PATCH "${CMAKE_MATCH_3}")
else()
    set(PROJECT_VERSION_MAJOR 0)
    set(PROJECT_VERSION_MINOR 0)
    set(PROJECT_VERSION_PATCH 0)
endif()

set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# Include full git describe in an extra variable
set(PROJECT_VERSION_FULL "${GIT_DESCRIBE}")

message(STATUS "Project version: ${PROJECT_VERSION}")
message(STATUS "Full Git describe: ${PROJECT_VERSION_FULL}")

#==================================

# set the project name and version
project(Pinggy VERSION ${PROJECT_VERSION})


# Specify the destination directory (base directory)
if(NOT PINGGY_RELEASE_DIR)
    set(PINGGY_RELEASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

add_subdirectory(src)
