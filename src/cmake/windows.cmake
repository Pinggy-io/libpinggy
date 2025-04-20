
set(PINGGY_OS "windows")

# Set architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCHITECTURE "x64")
    set(WIN_SSL_LIB_PATH_PREF "C:/Program Files/OpenSSL-Win64")
else()
    set(ARCHITECTURE "x86")
    set(WIN_SSL_LIB_PATH_PREF "C:/Program Files (x86)/OpenSSL-Win32")
endif()

if (NOT PINGGY_MSVC_RT)
    set(PINGGY_MSVC_RT "MTd" CACHE STRING "MSVC Runtime")
endif()

if(PINGGY_MSVC_RT STREQUAL "MTd")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug" CACHE STRING "" FORCE)
elseif(PINGGY_MSVC_RT STREQUAL "MDd")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL" CACHE STRING "" FORCE)
elseif(PINGGY_MSVC_RT STREQUAL "MT")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded" CACHE STRING "" FORCE)
elseif(PINGGY_MSVC_RT STREQUAL "MD")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "" FORCE)
else()
    message(FATAL_ERROR "Unsupported runtime.")
endif()

## Compiler options

target_compile_options(compiler_flags INTERFACE
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Werror;-Wunused;-Wno-unused-parameter;-Wno-missing-field-initializers;-Wno-ignored-qualifiers;-D_GNU_SOURCE>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)
target_compile_options(compiler_flags INTERFACE /wd4244 /wd5105 /wd4458 /wd4996 /wd4250 /wd4702 /wd4267 /EHsc)

include(cmake/commonFuncDef.cmake)


function(PlatformSpecificLinkings target mode)
    target_link_libraries(${target} ${mode} Ws2_32)
endfunction()

