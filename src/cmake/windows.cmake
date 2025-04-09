
set(PINGGY_OS "windows")

# Set architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCHITECTURE "x64")
    set(WIN_SSL_LIB_PATH_PREF "C:/Program Files/OpenSSL-Win64")
else()
    set(ARCHITECTURE "x86")
    set(WIN_SSL_LIB_PATH_PREF "C:/Program Files (x86)/OpenSSL-Win32")
endif()

## Compiler options

target_compile_options(compiler_flags INTERFACE
"$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Werror;-Wunused;-Wno-unused-parameter;-Wno-missing-field-initializers;-Wno-ignored-qualifiers;-D_GNU_SOURCE>>"
"$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)
target_compile_options(compiler_flags INTERFACE /wd4244 /wd5105 /wd4458 /wd4996 /wd4250 /wd4702 /wd4267 /EHsc)



if(BUILD_MODE_DLL STREQUAL "no")

    function(AddLibraryWithRuntime libName libType runtime)
        # message(STATUS "Linking ${libName}_${runtime} of type ${libType} with sources ${ARGN}")
        add_library(${libName}_${runtime} ${libType} ${ARGN})
        if (NOT libType STREQUAL "INTERFACE")
            target_compile_options(${libName}_${runtime} PRIVATE "/${runtimeFlag}")
            set_target_properties(${libName}_${runtime} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${runtime}/${libName}
                ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${runtime}/${libName}
            )
        endif()
    endfunction()


    function(AddExecutableWithRuntime target runtime)
        add_executable(${target}_${runtime} ${ARGN})
        target_compile_options(${target}_${runtime} PRIVATE "/${runtimeFlag}")
        set_target_properties(${target}_${runtime} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${runtime}/${target}
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${runtime}/${target}
        )
    endfunction()

    function(TargetLinkLibrariesWithRuntime target linkType runtime)
        if (NOT TARGET ${target}_${runtime})
            message(FATAL_ERROR "Target '${target}_${runtime}' does not exist.")
        endif()

        foreach(lib IN LISTS ARGN)
            # message(STATUS "Linking ${lib} to target ${target}")
            if (NOT TARGET ${lib}_${runtime})
                message(FATAL_ERROR "Target '${lib}_${runtime}' does not exist.")
            endif()
            target_link_libraries(${target}_${runtime} ${linkType} ${lib}_${runtime})
        endforeach()
    endfunction()


    function(TargetLinkLibrariesWithRuntimeOnlyTarget target linkType runtime)
        if (NOT TARGET ${target}_${runtime})
            message(FATAL_ERROR "Target '${target}_${runtime}' does not exist.")
        endif()

        foreach(lib IN LISTS ARGN)
            target_link_libraries(${target}_${runtime} ${linkType} ${lib})
        endforeach()
    endfunction()

    function(TargetIncludeDirectoriesWithRuntime target libType runtime)
        target_include_directories(${target}_${runtime} ${libType} ${ARGN})
    endfunction()

    function(AddLibrary libName libType)
        AddLibraryWithRuntime(${libName} ${libType} MD  ${ARGN})
        AddLibraryWithRuntime(${libName} ${libType} MDd ${ARGN})
        AddLibraryWithRuntime(${libName} ${libType} MT  ${ARGN})
        AddLibraryWithRuntime(${libName} ${libType} MTd ${ARGN})
    endfunction()


    function(TargetLinkLibraries target linkType)
        TargetLinkLibrariesWithRuntime(${target} ${linkType} MD  ${ARGN})
        TargetLinkLibrariesWithRuntime(${target} ${linkType} MDd ${ARGN})
        TargetLinkLibrariesWithRuntime(${target} ${linkType} MT  ${ARGN})
        TargetLinkLibrariesWithRuntime(${target} ${linkType} MTd ${ARGN})
    endfunction()

    function(TargetLinkLibrariesOnlyTarget target linkType)
        TargetLinkLibrariesWithRuntimeOnlyTarget(${target} ${linkType} MD  ${ARGN})
        TargetLinkLibrariesWithRuntimeOnlyTarget(${target} ${linkType} MDd ${ARGN})
        TargetLinkLibrariesWithRuntimeOnlyTarget(${target} ${linkType} MT  ${ARGN})
        TargetLinkLibrariesWithRuntimeOnlyTarget(${target} ${linkType} MTd ${ARGN})
    endfunction()

    function(TargetIncludeDirectories target libType)
        TargetIncludeDirectoriesWithRuntime(${target} ${libType} MD  ${ARGN})
        TargetIncludeDirectoriesWithRuntime(${target} ${libType} MDd ${ARGN})
        TargetIncludeDirectoriesWithRuntime(${target} ${libType} MT  ${ARGN})
        TargetIncludeDirectoriesWithRuntime(${target} ${libType} MTd ${ARGN})
    endfunction()

    function(AddExecutable target)
        AddExecutableWithRuntime(${target} MD  ${ARGN})
        AddExecutableWithRuntime(${target} MDd ${ARGN})
        AddExecutableWithRuntime(${target} MT  ${ARGN})
        AddExecutableWithRuntime(${target} MTd ${ARGN})
    endfunction()

    function(AddTargetCopyCommandFinal target toPath cmnt)
        message(STATUS "Copy ${target} to ${toPath} with cmnt `${cmnt}`")
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> ${toPath}
                COMMENT ${cmnt}
        )
    endfunction()

    function(AddTargetCopyCommand target toPath cmnt)
        AddTargetCopyCommandFinal(${target}_MD  ${toPath} ${cmnt})
        AddTargetCopyCommandFinal(${target}_MDd ${toPath} ${cmnt})
        AddTargetCopyCommandFinal(${target}_MT  ${toPath} ${cmnt})
        AddTargetCopyCommandFinal(${target}_MTd ${toPath} ${cmnt})
    endfunction()

    #===============================
    function(SetupLSSLRuntime target runtime)
        # add_library(lssl_${runtime} INTERFACE)
        target_include_directories(${target}_${runtime} PUBLIC "${WIN_SSL_LIB_PATH_PREF}/include")
        target_link_directories(${target}_${runtime} PUBLIC "${WIN_SSL_LIB_PATH_PREF}/lib/VC/${ARCHITECTURE}/${runtime}")
        target_link_libraries(${target}_${runtime} PUBLIC libssl_static libcrypto_static)
    endfunction()

    function(LinkSSL target)
        SetupLSSLRuntime(${target} MD )
        SetupLSSLRuntime(${target} MDd)
        SetupLSSLRuntime(${target} MT )
        SetupLSSLRuntime(${target} MTd)
    endfunction()
    #===============================
    # function(SetupLSSLRuntime runtime)
    #     add_library(lssl_${runtime} INTERFACE)
    #     target_include_directories(lssl_${runtime} INTERFACE "${WIN_SSL_LIB_PATH_PREF}/include")
    #     target_link_directories(lssl_${runtime} INTERFACE "${WIN_SSL_LIB_PATH_PREF}/lib/VC/${ARCHITECTURE}/${runtime}")
    #     # target_link_libraries(lssl_${runtime} PUBLIC libssl libcrypto)
    # endfunction()

    # function(SetupLSSL)
    #     SetupLSSLRuntime(MD )
    #     SetupLSSLRuntime(MDd)
    #     SetupLSSLRuntime(MT )
    #     SetupLSSLRuntime(MTd)
    # endfunction()

    # function(LinkSSL target)
    #     target_link_libraries(${target}_MD  PUBLIC libssl_static libcrypto_static)
    #     target_link_libraries(${target}_MDd PUBLIC libssl_static libcrypto_static)
    #     target_link_libraries(${target}_MT  PUBLIC libssl_static libcrypto_static)
    #     target_link_libraries(${target}_MTd PUBLIC libssl_static libcrypto_static)
    # endfunction()


    # SetupLSSL()
    #===============================

    AddLibrary(compiler_flags INTERFACE)
    TargetLinkLibrariesOnlyTarget(compiler_flags INTERFACE compiler_flags)

else()

include(cmake/commonFuncDef.cmake)


function(PlatformSpecificLinkings target mode)
    target_link_libraries(${target} ${mode} Ws2_32)
endfunction()

endif()
