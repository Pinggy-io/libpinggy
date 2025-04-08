
set(PINGGYSDK_TUNNEL_SOURCES
    Sdk.cc
    SdkChannelWraper.cc
)

AddLibrary(pinggysdk STATIC ${PINGGYSDK_TUNNEL_SOURCES})

TargetIncludeDirectories(pinggysdk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TargetLinkLibraries(pinggysdk PUBLIC protocol)
PlatformSpecificLinkings(pinggysdk PUBLIC)

set(ARCHIVE_TARGETS)

#=====================
if(BUILD_MODE_STATIC STREQUAL "yes")
    AddLibrary(pinggyapi_static STATIC Pinggy_c.cc)
    set_target_properties(pinggyapi_static PROPERTIES OUTPUT_NAME "pinggy")
    EnableSymbolExport(pinggyapi_static __NOEXPORT_PINGGY_DLL__)
    AddPlatFormHeader(pinggyapi_static)

    TargetLinkLibraries(pinggyapi_static PUBLIC pinggysdk)
    LinkSSL(pinggyapi_static)
    AddCopyTarget(copyPinggyApi_static pinggyapi_static ${PINGGY_RELEASE_DIR} "Copying sdk to base dir")
    list(APPEND ARCHIVE_TARGETS $<TARGET_FILE:pinggyapi_static>)

    install(TARGETS pinggyapi_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

    set(CopyHeaderTarget copyPinggyApi_static)


    # Call the recursive function for a target
    ListRecursiveStaticLibraries(pinggyapi_static pinggyapi_static)
endif()

#===============

if(NOT BUILD_MODE_DLL STREQUAL "no")
    AddLibrary(pinggyapi_shared SHARED Pinggy_c.cc)
    set_target_properties(pinggyapi_shared PROPERTIES OUTPUT_NAME "pinggy")
    EnableSymbolExport(pinggyapi_shared __EXPORT_PINGGY_DLL__)
    AddPlatFormHeader(pinggyapi_shared)

    TargetLinkLibraries(pinggyapi_shared PUBLIC pinggysdk)
    LinkSSL(pinggyapi_shared)
    AddCopyTarget(copyPinggyApi_shared pinggyapi_shared ${PINGGY_RELEASE_DIR} "Copying sdk to base dir")

    install(TARGETS pinggyapi_shared
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

    set(CopyHeaderTarget copyPinggyApi_shared)
    AddSslCopyTarget(copyOpenSsl_shared copyPinggyApi_shared ${PINGGY_RELEASE_DIR} "Copying openssl to base dir")
    list(APPEND ARCHIVE_TARGETS $<TARGET_FILE:pinggyapi_shared>)
endif()

if(NOT PINGGY_HEADER_RELEASE_DIR)
    set(PINGGY_HEADER_RELEASE_DIR ${PINGGY_RELEASE_DIR})
endif()

if(CopyHeaderTarget)
    message("The header release target is: ${PINGGY_HEADER_RELEASE_DIR}")
    if(PINGGY_HEADER_RELEASE_DIR)
        CopyHeaders(copyHeader ${CopyHeaderTarget} pinggy.h ${PINGGY_HEADER_RELEASE_DIR})
    endif()
    list(APPEND ARCHIVE_TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/pinggy.h)
endif()



if(NOT PINGGY_ARCHIVE_RELEASE_DIR)
    set(PINGGY_ARCHIVE_RELEASE_DIR ${PINGGY_RELEASE_DIR})
endif()

DistributeLibPinggy(libpinggy ${PINGGY_ARCHIVE_RELEASE_DIR} ${ARCHIVE_TARGETS})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pinggy.h
    DESTINATION include/pinggy/)
