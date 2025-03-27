


# Common sources
set(SOURCES
    PinggyPoll.cc
    PinggyPollGeneric.cc
    PollableFD.cc
)

# Platform-specific sources
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND SOURCES
        PinggyPollMac.cc
        ThreadPool.cc
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SOURCES
        PinggyPollLinux.cc
        ThreadPool.cc
    )
# elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
# elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
endif()

AddLibrary(poll
            STATIC
            ${SOURCES}
)

TargetIncludeDirectories(poll
            PUBLIC
            ${PINGGY_COMMON_SOURCE_DIR}
)

TargetLinkLibraries(poll
            PUBLIC
            compiler_flags
            utils
)
