
add_custom_target(
    releaselib
    COMMENT "Packaging complete"
)


add_custom_target(
    releasessl
    COMMENT "Packaging complete"
)

if(NOT PINGGY_BUILD_ARCH)
    set(PINGGY_BUILD_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

function(AddLibrary lib_name lib_type)
    add_library(${lib_name} ${lib_type} ${ARGN})
endfunction()

function(TargetLinkLibraries target linkType)
    if (NOT TARGET ${target})
        message(FATAL_ERROR "Target '${target}' does not exist.")
    endif()

    foreach(lib IN LISTS ARGN)
        target_link_libraries(${target} ${linkType} ${lib})
    endforeach()
endfunction()

function(TargetLinkLibrariesOnlyTarget target linkType)
    TargetLinkLibraries(${target} ${linkType} ${ARGN})
endfunction()

function(TargetIncludeDirectories target libType)
    target_include_directories(${target} ${libType} ${ARGN})
endfunction()

function(AddExecutable target exeType)
    add_executable(${target} ${exeType} ${ARGN})
endfunction()

function(AddCopyTarget target anotherTarget dest cmnt)
    add_custom_target(${target}
        ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${anotherTarget}> ${dest}
        DEPENDS ${anotherTarget}
        COMMENT ${cmnt}
    )
    add_dependencies(releaselib ${target})
endfunction()

function(CopyHeaders target dependecy srcFile destDir)
    add_custom_target(${target}
        ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${srcFile}" ${destDir}
        DEPENDS ${dependecy}
        COMMENT ${cmnt}
    )
    add_dependencies(releaselib ${target})
endfunction()


function(SetupSSL)
    find_package(OpenSSL 3.0.3)
    if (OPENSSL_FOUND)
        # On CMake < 3.16, OPENSSL_CRYPTO_LIBRARIES is usually a synonym for OPENSSL_CRYPTO_LIBRARY, but is not defined
        # when building on Windows outside of Cygwin. We provide the synonym here, if FindOpenSSL didn't define it already.
        if (NOT DEFINED OPENSSL_CRYPTO_LIBRARIES)
            set(OPENSSL_CRYPTO_LIBRARIES ${OPENSSL_CRYPTO_LIBRARY})
        endif()

        set(OPENSSL_INCLUDE_DIR "${OPENSSL_INCLUDE_DIR}" PARENT_SCOPE)
        set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_CRYPTO_LIBRARY}" PARENT_SCOPE)
        set(OPENSSL_CRYPTO_LIBRARIES "${OPENSSL_CRYPTO_LIBRARIES}" PARENT_SCOPE)
        set(OPENSSL_SSL_LIBRARY "${OPENSSL_SSL_LIBRARY}" PARENT_SCOPE)
        set(OPENSSL_SSL_LIBRARIES "${OPENSSL_SSL_LIBRARIES}" PARENT_SCOPE)
        set(OPENSSL_LIBRARIES "${OPENSSL_LIBRARIES}" PARENT_SCOPE)
        set(OPENSSL_VERSION "${OPENSSL_VERSION}" PARENT_SCOPE)
        set(OPENSSL_APPLINK_SOURCE "${OPENSSL_APPLINK_SOURCE}" PARENT_SCOPE)

    else()
        message(FATAL_ERROR "Target '${OPENSSL_FOUND}' does not exist.")
    endif()
endfunction()

function(IncludeSSL target)
    message(STATUS "Including ${OPENSSL_INCLUDE_DIR} for openssl")
    target_include_directories(${target} PUBLIC ${OPENSSL_INCLUDE_DIR} )
endfunction()

function(LinkSSL target)
    # message(STATUS "Linking OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR}")
    # message(STATUS "Linking OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR}")
    # message(STATUS "Linking OPENSSL_CRYPTO_LIBRARY ${OPENSSL_CRYPTO_LIBRARY}")
    # message(STATUS "Linking OPENSSL_CRYPTO_LIBRARIES ${OPENSSL_CRYPTO_LIBRARIES}")
    # message(STATUS "Linking OPENSSL_SSL_LIBRARY ${OPENSSL_SSL_LIBRARY}")
    # message(STATUS "Linking OPENSSL_SSL_LIBRARIES ${OPENSSL_SSL_LIBRARIES}")
    # message(STATUS "Linking OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES}")
    # message(STATUS "Linking OPENSSL_VERSION ${OPENSSL_VERSION}")
    # message(STATUS "Linking OPENSSL_APPLINK_SOURCE ${OPENSSL_APPLINK_SOURCE}")
    target_link_libraries(${target}  PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    set_target_properties(${target} PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@loader_path/openssl/lib"  # or "@rpath"
    )
endfunction()

function(EnableSymbolExport target)
    target_compile_definitions(${target} PRIVATE ${ARGN})
endfunction()

SetupSSL()

function(AddSslCopyTarget target anotherTarget dest cmnt)
    if(DEFINED OPENSSL_ROOT_DIR)
        add_custom_target(${target}
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${OPENSSL_ROOT_DIR}"  # Source
                "${dest}/openssl" # Destination
                DEPENDS ${anotherTarget}
                COMMENT ${cmnt}
        )
        add_dependencies(releasessl ${target})
    endif()
endfunction()

#=================================
function(DistributeLibPinggy libname dest)
    set(DIST_STAGE ${CMAKE_BINARY_DIR}/dist_stage)

    if(WIN32)
        set(ARCHIVE_NAME_dirty ${dest}/${libname}-${PINGGY_OS}-${PINGGY_BUILD_ARCH}.zip)
        set(WITH_SSL_ARCHIVE_NAME_dirty ${dest}/${libname}-ssl-${PINGGY_OS}-${PINGGY_BUILD_ARCH}.zip)
        set(ARCHIVE_FORMAT --format=zip)
    else()
        set(ARCHIVE_NAME_dirty ${dest}/${libname}-${PINGGY_OS}-${PINGGY_BUILD_ARCH}.tgz)
        set(WITH_SSL_ARCHIVE_NAME_dirty ${dest}/${libname}-ssl-${PINGGY_OS}-${PINGGY_BUILD_ARCH}.tgz)
        set(ARCHIVE_FORMAT --format=gnutar)
    endif()

    file(TO_NATIVE_PATH "${ARCHIVE_NAME_dirty}" ARCHIVE_NAME)
    file(TO_NATIVE_PATH "${WITH_SSL_ARCHIVE_NAME_dirty}" WITH_SSL_ARCHIVE_NAME)

    set(FILES_TO_COPY "")
    set(FILES_TO_ARCHIVE "")

    foreach(item IN LISTS ARGN)
        # Try to treat as target first
        if(TARGET ${item})
            # This generator expression will be evaluated at build time
            set(out "$<TARGET_FILE:${item}>")
            set(base "$<TARGET_FILE_NAME:${item}>")
        else()
            set(out "${item}")
            file(TO_CMAKE_PATH "${out}" norm)
            get_filename_component(base "${norm}" NAME)
        endif()

        list(APPEND FILES_TO_COPY "${out}")
        list(APPEND FILES_TO_ARCHIVE "${base}")
    endforeach()

    # Copy files to dist_stage
    add_custom_command(
        OUTPUT ${DIST_STAGE}/.stamp
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${DIST_STAGE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_STAGE}
        COMMAND ${CMAKE_COMMAND} -E copy ${FILES_TO_COPY} ${DIST_STAGE}
        COMMAND ${CMAKE_COMMAND} -E touch ${DIST_STAGE}/.stamp
        COMMENT "Staging files for distribution"
    )

    # Create archive
    add_custom_command(
        OUTPUT ${ARCHIVE_NAME}
        DEPENDS ${DIST_STAGE}/.stamp
        WORKING_DIRECTORY ${DIST_STAGE}
        COMMAND ${CMAKE_COMMAND} -E tar "cfv" ${ARCHIVE_NAME} ${ARCHIVE_FORMAT} -- ${FILES_TO_ARCHIVE}
        COMMENT "Creating archive ${ARCHIVE_NAME}"
    )

    if(DEFINED OPENSSL_ROOT_DIR)
        list(APPEND FILES_TO_ARCHIVE "openssl")
        add_custom_command(
            OUTPUT ${DIST_STAGE}/openssl
            DEPENDS ${ARCHIVE_NAME}
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${OPENSSL_ROOT_DIR}" ${DIST_STAGE}/openssl
            COMMENT "coping ${OPENSSL_ROOT_DIR} ${DIST_STAGE}/openssl"
        )

        add_custom_command(
            OUTPUT ${WITH_SSL_ARCHIVE_NAME}
            DEPENDS ${DIST_STAGE}/openssl
            WORKING_DIRECTORY ${DIST_STAGE}
            COMMAND ${CMAKE_COMMAND} -E tar "cfv" ${WITH_SSL_ARCHIVE_NAME} ${ARCHIVE_FORMAT} -- ${FILES_TO_ARCHIVE}
            COMMENT "Creating archive ${WITH_SSL_ARCHIVE_NAME}"
        )

        add_custom_target(distribute
            DEPENDS ${WITH_SSL_ARCHIVE_NAME}
            COMMENT "Packaging complete"
        )
    else()
        add_custom_target(distribute
            DEPENDS ${ARCHIVE_NAME}
            COMMENT "Packaging complete"
        )
    endif()

    # Final target
    if(DEFINED OPENSSL_ROOT_DIR)
    else()
    endif()
endfunction()

#=================================

function(ListRecursiveStaticLibraries target origTarget)
    # Prevent infinite recursion by checking visited targets

    # Get all libraries linked to the target
    get_target_property(linked_libraries ${target} INTERFACE_LINK_LIBRARIES)

    # If there are no linked libraries, return
    if(NOT linked_libraries)
        return()
    endif()


    # Process each linked library
    foreach(lib ${linked_libraries})

        if(TARGET ${lib})
            # Check the type of the target
            get_target_property(lib_type ${lib} TYPE)
            if(lib_type STREQUAL "STATIC_LIBRARY")
                get_target_property(SOURCES_A ${origTarget} SOURCES)

                target_sources(${origTarget} PRIVATE $<TARGET_OBJECTS:${lib}>)
                # Recurse into the dependency
                ListRecursiveStaticLibraries(${lib} ${origTarget})
            endif()
        endif()

        if(TARGET ${lib})
            get_target_property(lib_type ${lib} TYPE)
            if(lib_type STREQUAL "STATIC_LIBRARY")
            endif()
        endif()
    endforeach()

endfunction()
