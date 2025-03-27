
set(SOURCES
        Log.cc
        network.c
        networkaddresses.c
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND SOURCES
        platform_win.c
    )
else()
    list(APPEND SOURCES
        platform_nix.c
    )
endif()

AddLibrary(platform
            STATIC
            ${SOURCES}
)

TargetIncludeDirectories(platform
            PUBLIC
            ${PINGGY_COMMON_SOURCE_DIR}
)

TargetLinkLibraries(platform
            PUBLIC
            compiler_flags
)
