# Git commit ID
find_package(Git REQUIRED)
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    OUTPUT_VARIABLE GIT_COMMIT_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Timestamp
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

# Libc Version Detection
if (APPLE)
    execute_process(
        COMMAND uname -v
        OUTPUT_VARIABLE LIBC_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
elseif (CYGWIN)
    execute_process(
        COMMAND uname -srm
        OUTPUT_VARIABLE LIBC_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
elseif (MINGW)
    execute_process(
        COMMAND gcc -dumpversion
        OUTPUT_VARIABLE LIBC_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
elseif (UNIX AND NOT APPLE)
    execute_process(
        COMMAND ldd --version
        OUTPUT_VARIABLE LIBC_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REPLACE "\n" " " LIBC_VERSION "${LIBC_VERSION}")
elseif (MSVC)
    set(LIBC_VERSION "MSVC Runtime")
else()
    set(LIBC_VERSION "Unknown")
endif()

# OS Information
if (CYGWIN)
    execute_process(
        COMMAND uname -srm
        OUTPUT_VARIABLE BUILD_OS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
elseif (MINGW)
    set(BUILD_OS "MinGW Windows")
elseif (WIN32)
    set(BUILD_OS "Windows")
else()
    execute_process(
        COMMAND uname -srm
        OUTPUT_VARIABLE BUILD_OS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

# # Configure file
# configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)

configure_file(platform_config.h.in platform_config.h @ONLY)

function(AddPlatFormHeader target)
    if (MSVC)
        target_compile_options(${target} PRIVATE /FI"${PINGGY_TOP_SOURCE_DIR}/platform_config.h")
    else()
        target_compile_options(${target} PRIVATE -include "${PINGGY_TOP_SOURCE_DIR}/platform_config.h")
    endif()
endfunction()