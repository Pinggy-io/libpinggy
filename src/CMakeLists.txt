

# Specify the destination directory (base directory)
if(NOT PINGGY_RELEASE_DIR)
    set(PINGGY_RELEASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

message(STATUS "CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH}")
message(STATUS "CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}")

#=====================================================

# Specify the destination directory (base directory)
if(NOT PINGGY_RELEASE_DIR)
    set(PINGGY_RELEASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

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
configure_file(platform_config.h.in platform_config.h)


# AddSubdirectories("*")

add_subdirectory(third-party)
add_subdirectory(common)
add_subdirectory(protocol)
add_subdirectory(sdk)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # if(BUILD_MODE_DLL STREQUAL "no")
    add_subdirectory(client)
    # endif()
endif()

