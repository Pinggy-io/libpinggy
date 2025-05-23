

# Specify the destination directory (base directory)
if(NOT PINGGY_RELEASE_DIR)
    set(PINGGY_RELEASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(NOT PINGGY_BUILD_PINGGY_CLIENT)
    set(PINGGY_BUILD_PINGGY_CLIENT "no")
endif()

set(PINGGY_TOP_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH}")
message(STATUS "CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}")

#=====================================================

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

#=====================================================

include(cmake/common.cmake)


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(cmake/linux.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    include(cmake/macos.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(cmake/windows.cmake)
else()
    message(FATAL_ERROR "Unsupported operating system.")
endif()
#=====================================================
#=====================================================

# configure a header file to pass some of the CMake settings
# to the source code
include(cmake/platform_config.cmake)
#=====================================================


# AddSubdirectories("*")

add_subdirectory(third-party)
add_subdirectory(common)
add_subdirectory(protocol)
add_subdirectory(sdk)

# if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
#     if(NOT PINGGY_BUILD_PINGGY_CLIENT STREQUAL "no")
        add_subdirectory(client)
#     endif()
# endif()

