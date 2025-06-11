
# Common sources
set(SOURCES
    AddressCache.cc
    ConnectionListener.cc
    DummyConnection.cc
    NetworkConnection.cc
    SslConnectionListener.cc
    SslNetworkConnection.cc
    UdpConnection.cc
    SslNetConnBio.cc
)




AddLibrary(net
            STATIC
            ${SOURCES}
)

TargetIncludeDirectories(net
    PUBLIC
    ${PINGGY_COMMON_SOURCE_DIR}
)

TargetLinkLibraries(net
            PUBLIC
            compiler_flags
            poll
)
IncludeSSL(net)
LinkSSL(net)
