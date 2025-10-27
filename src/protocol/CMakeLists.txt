
set(SOURCES
    Channel.cc
    ChannelConnectionForwarder.cc
    Schema.cc
    Session.cc
    SessionFeatures.cc

    transport/Deserialization.cc
    transport/PathRegistry.cc
    transport/Serialization.cc
    transport/TransportManager.cc
    transport/PinggyValue.cc
)

AddLibrary(protocol
            STATIC
            ${SOURCES}
)

TargetIncludeDirectories(protocol
                PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}
)
TargetLinkLibraries(protocol
                PUBLIC
                net
)

