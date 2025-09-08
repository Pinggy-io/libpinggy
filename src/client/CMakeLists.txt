
set(SOURCES
    ClientMain.cc
)

list(APPEND SOURCES
    cli_getopt.c
)

AddExecutable(pinggyclient ${SOURCES})

AddPlatFormHeader(pinggyclient)

TargetLinkLibraries(pinggyclient PRIVATE pinggysdk)

AddCopyClient(pinggyclientcopy pinggyclient ${PINGGY_RELEASE_DIR} "Copying pinggyclient to the base directory")

if(BUILD_MODE_STATIC STREQUAL "yes")

    set(CSOURCES
        client_main.c
    )

    list(APPEND CSOURCES
        cli_getopt.c
    )

    AddExecutable(pinggyclientc ${CSOURCES})

    AddPlatFormHeader(pinggyclientc)

    TargetLinkLibraries(pinggyclientc PRIVATE pinggysdk)

    AddCopyClient(pinggyclientccopy pinggyclientc ${PINGGY_RELEASE_DIR} "Copying pinggyclientc to the base directory")

endif()