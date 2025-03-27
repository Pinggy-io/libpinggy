
include(cmake/commonFuncDef.cmake)

function(PlatformSpecificLinkings target)
    #Empty body
endfunction()


## Set libraries

function(SetupSSH)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBSSH REQUIRED libssh>=0.11.0)
    if(LIBSSH_LIBRARIES AND LIBSSH_INCLUDE_DIRS)
        message(STATUS "ssh found (${LIBSSH_LIBRARIES}) ${LIBSSH_INCLUDE_DIRS} `${LIBSSH_LINK_LIBRARIES}` `${LIBSSH_LIBRARY_DIRS}`")
    else()
        message(FATAL_ERROR "No suitable libssh found")
    endif()
endfunction()

function(IncludeSSH target)
    # message(STATUS "Including `${ssh_INCLUDE_DIRS}` `${ssh_INCLUDE_DIR}`")
    target_link_directories(${target} PUBLIC ${LIBSSH_LIBRARY_DIRS})
    target_include_directories(${target} PUBLIC ${LIBSSH_INCLUDE_DIRS})
endfunction()

function(LinkSSH target)
    # TargetLinkLibraries(${target} PRIVATE lssl)
    target_link_libraries(${target}  PRIVATE ${LIBSSH_LIBRARIES})
endfunction()

SetupSSH()

if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
else()
    message(STATUS "CMAKE_OSX_DEPLOYMENT_TARGET set to ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()

## Compiler options
target_compile_options(compiler_flags INTERFACE
"$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Werror;-Wunused;-Wno-unused-parameter;-Wno-missing-field-initializers;-Wno-ignored-qualifiers;-Wno-undef-prefix>>"
)

target_link_libraries(compiler_flags INTERFACE
    "-framework Foundation"
)

