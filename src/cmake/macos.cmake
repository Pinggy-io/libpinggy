
include(cmake/commonFuncDef.cmake)

function(PlatformSpecificLinkings target)
    #Empty body
endfunction()


## Set libraries

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

