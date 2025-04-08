
## set compiler
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")

# f(CMAKE_SIZEOF_VOID_P EQUAL 4)
#    set(ARCH_FLAGS "-m32")
# lseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
#    set(ARCH_FLAGS "-m64")
# ndif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH_FLAGS}")

if(NOT CMAKE_CXX_COMPILER)
    option(USE_GCC "Use GCC as the compiler" OFF)

    find_program(CLANG_EXECUTABLE NAMES clang++ clang)
    find_program(GCC_EXECUTABLE NAMES g++ gcc)

    if(USE_GCC AND GCC_EXECUTABLE)
        message(STATUS "GCC found: ${GCC_EXECUTABLE}")
        set(CMAKE_CXX_COMPILER ${GCC_EXECUTABLE})
    else()
        if(CLANG_EXECUTABLE)
            message(STATUS "Clang found: ${CLANG_EXECUTABLE}")
            set(CMAKE_CXX_COMPILER ${CLANG_EXECUTABLE})
        else()
            message(STATUS "Clang not found, checking for GCC")
            if(GCC_EXECUTABLE)
                message(STATUS "GCC found: ${GCC_EXECUTABLE}")
                set(CMAKE_CXX_COMPILER ${GCC_EXECUTABLE})
            else()
                message(FATAL_ERROR "No suitable C++ compiler found")
            endif()
        endif()
    endif()
endif()


include(cmake/commonFuncDef.cmake)

function(PlatformSpecificLinkings target)

endfunction()


## Set libraries

## Compiler options

target_compile_options(compiler_flags INTERFACE
"$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Werror;-Wunused;-Wno-unused-parameter;-Wno-missing-field-initializers;-Wno-ignored-qualifiers;-D_GNU_SOURCE;>>"
"$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)


# # cmake_policy(SET CMP0148 NEW)

# function(GenerateSwigPython target source depend)
#     find_package(SWIG REQUIRED)

#     message(STATUS "SWIG_FOUND ${SWIG_FOUND}")
#     message(STATUS "SWIG_EXECUTABLE ${SWIG_EXECUTABLE}")
#     message(STATUS "SWIG_DIR ${SWIG_DIR}")
#     message(STATUS "SWIG_VERSION ${SWIG_VERSION}")

#     find_package(Python COMPONENTS Interpreter Development)
#     include(UseSWIG)

#     set_source_files_properties(${source} PROPERTIES
#         CPLUSPLUS ON
#     )

#     swig_add_library(${target}
#         TYPE SHARED
#         LANGUAGE python
#         SOURCES ${source}
#     )


#     get_property(support_files TARGET ${target} PROPERTY GENERATED_COMPILE_OPTIONS)

#     message(STATUS "1 ${depend} ${target} ${SWIG_MODULE_${target}_REAL_NAME} : ${support_files} : ${Python_INCLUDE_DIRS} : ${Python_LIBRARIES}")

#     target_include_directories(${target} PRIVATE
#         ${PYTHON_INCLUDE_PATH}
#         ${Python_INCLUDE_DIRS}
#     )

#     target_link_libraries(${target}
#         PRIVATE Python::Python
#     )
#     target_link_libraries(${target} PRIVATE ${depend})

#     # set_target_properties(${target} SWIG_DEPENDS ${depend})

# endfunction()


# function(GenerateSwigPythonOld target)
#     # This is a CMake example for Python

#     find_package(SWIG REQUIRED)
#     include(UseSWIG)

#     find_package(PythonLibs)
#     include_directories(${PYTHON_INCLUDE_PATH})
#     include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#     set_property(SOURCE ${target}.i PROPERTY CPLUSPLUS ON)
#     swig_add_library(${target} TYPE SHARED LANGUAGE python SOURCES ${target}.i)
#     # target_link_libraries(${target} PRIVATE ${ARGN})

#     target_include_directories(${SWIG_MODULE_${target}_REAL_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
#     target_link_libraries(${SWIG_MODULE_${target}_REAL_NAME} PRIVATE ${ARGN})

#     # include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#     # set(CMAKE_SWIG_FLAGS "")

#     # set_source_files_properties(${target}.i PROPERTIES CPLUSPLUS ON)
#     # swig_add_library(${target} TYPE MODULE LANGUAGE python SOURCES ${target}.i)
#     # swig_link_libraries(${target} ${PYTHON_LIBRARIES} ${ARGN})
# endfunction()


# # Function to add a SWIG module with a CMake static library
# function(add_swig_module_with_lib module_name i_file header_file lib_target)
#     # Find SWIG and Python
#     find_package(SWIG REQUIRED)
#     find_package(Python COMPONENTS Interpreter Development REQUIRED)
#     include(UseSWIG)

#     # Set properties for the SWIG interface file
#     set_source_files_properties(${i_file} PROPERTIES CPLUSPLUS ON)

#     # Add the SWIG module
#     swig_add_library(${module_name} TYPE MODULE LANGUAGE python SOURCES ${i_file})

#     # Link the SWIG module with the CMake static library and Python
#     target_include_directories(${SWIG_MODULE_${module_name}_REAL_NAME} PRIVATE
#         ${CMAKE_SOURCE_DIR}
#         ${Python_INCLUDE_DIRS}  # Add Python include directory
#     )
#     target_link_libraries(${SWIG_MODULE_${module_name}_REAL_NAME} PRIVATE
#         ${lib_target}           # Link with the static library target
#         Python::Python          # Link with Python libraries
#     )

#     # Specify additional dependencies if required
#     set(SWIG_MODULE_${module_name}_EXTRA_DEPS ${header_file})
# endfunction()
