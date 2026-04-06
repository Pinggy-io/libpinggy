
cmake_minimum_required(VERSION 3.15)


find_package(Git)

#==================================
# Get full version string: tag + commits since + short hash
if(NOT DEFINED GIT_DESCRIBE)
    # Primary source of truth: git describe
    if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_DESCRIBE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
    endif()

    # Fail-safe: Try to read the version injected by git archive (GitHub releases)
    if(NOT GIT_DESCRIBE AND EXISTS "${CMAKE_SOURCE_DIR}/version.txt")
        file(READ "${CMAKE_SOURCE_DIR}/version.txt" VERSION_FILE_CONTENT)
        string(STRIP "${VERSION_FILE_CONTENT}" VERSION_FILE_CONTENT)
        # Only use it if git actually substituted the placeholder
        if(NOT VERSION_FILE_CONTENT MATCHES "^\\$Format:")
            set(GIT_DESCRIBE "${VERSION_FILE_CONTENT}")
        endif()
    endif()

    if(NOT GIT_DESCRIBE)
        set(GIT_DESCRIBE "0.0.0") # Fallback version for source archives
    endif()
endif()

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
