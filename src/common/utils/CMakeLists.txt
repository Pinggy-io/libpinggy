
AddLibrary(utils
            STATIC
            Utils.cc
            CertificateFileDetail.cc
            RawData.cc
            StringUtils.cc
            Semaphore.cc
)


TargetIncludeDirectories(utils
            PUBLIC
            ${PINGGY_COMMON_SOURCE_DIR}
)

TargetLinkLibraries(utils
            PUBLIC
            compiler_flags
            platform
            pub_thirdparty
)
