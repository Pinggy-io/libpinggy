
set(SOURCES
    ClientMain.cc
)

list(APPEND SOURCES
    cli_getopt.c
)


AddExecutable(pinggyclient ${SOURCES})

TargetLinkLibraries(pinggyclient PRIVATE pinggysdk)

AddCopyClient(pinggyclientcopy pinggyclient ${PINGGY_RELEASE_DIR} "Copying pinggyclient to the base directory")

# LinkSSL(pinggyclient)
