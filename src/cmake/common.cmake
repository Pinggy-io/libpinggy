# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(AddSubdirectories pattern)
    # Use the current source directory as the base directory
    set(baseDir ${CMAKE_CURRENT_SOURCE_DIR})

    # Get all subdirectories matching the pattern
    file(GLOB subdirs RELATIVE ${baseDir} ${baseDir}/${pattern})

    # For each directory, add it as a subdirectory
    foreach(subdir ${subdirs})
        if(IS_DIRECTORY ${baseDir}/${subdir})
            add_subdirectory(${baseDir}/${subdir})
        endif()
    endforeach()
endfunction()

# if(NOT BUILD_MODE_DLL STREQUAL "yes")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
endif()
# endif()



#===============

option(LOG_LEVEL "Set compile time log level" LogLevelDebug)

if(NOT LOG_LEVEL)
    set(LOG_LEVEL LogLevelDebug)
endif()


# Check if using MSVC
if(MSVC)
    set(PREPROCESS_FLAG /P)  # MSVC uses /P for preprocessing
else()
    set(PREPROCESS_FLAG -E)   # GCC/Clang use -E for preprocessing
endif()

#=================

add_library(compiler_flags INTERFACE)
# target_compile_features(compiler_flags INTERFACE cxx_std_17)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

target_compile_options(compiler_flags INTERFACE -DLOG_LEVEL=${LOG_LEVEL})

# set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DLOG_LEVEL=${LOG_LEVEL}")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DLOG_LEVEL=${LOG_LEVEL}")
