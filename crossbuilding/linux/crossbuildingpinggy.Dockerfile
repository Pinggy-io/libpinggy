# FROM ubuntu:20.04

# RUN apt-get update -y
# RUN apt-get install -y gcc-9-arm-linux-gnueabihf gcc-9-multilib-arm-linux-gnueabihf binutils-arm-linux-gnueabihf libgcc1-armhf-cross libsfgcc1-armhf-cross libstdc++6-armhf-cross

# Use Ubuntu 20.04 as the base image
FROM ubuntu:20.04

# Set environment variables to avoid interactive prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Update and install necessary packages
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    wget \
    cmake \
    git \
    libstdc++-10-dev \
    libgcc-10-dev \
    libstdc++-9-dev-arm64-cross \
    libstdc++-9-dev-armhf-cross \
    libstdc++6-arm64-cross \
    libstdc++6-armhf-cross \
    crossbuild-essential-armhf \
    crossbuild-essential-arm64 \
    gcc-9-multilib-i686-linux-gnu \
    g++-9-multilib-i686-linux-gnu \
    mingw-w64 \
    vim && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set environment variables for cross-compiling
ENV TARGET_ARCH_armv7=arm-linux-gnueabihf
ENV TARGET_ARCH_aarch64=aarch64-linux-gnu
ENV TARGET_ARCH_i686=i686-linux-gnu
ENV TARGET_ARCH_mingw=i686-w64-mingw32
ENV TARGET_ARCH_mingw64=x86_64-w64-mingw32
ENV OPENSSL_VERSION=3.3.1

RUN printf "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n"   \
    "TARGET=${TARGET_ARCH_armv7}"               \
    "export CC=\${TARGET}-gcc"                  \
    "export CXX=\${TARGET}-g++"                 \
    "export AR=\${TARGET}-ar"                   \
    "export RUNLIB=\${TARGET}-runlib"           \
    "export LD=\${TARGET}-ld"                   \
    "export ARCH=armv7"                         \
    "export OPENSSL_ROOT_PATH=/opt/openssl/arm" \
        > /opt/setup-armv7.sh

RUN printf "%s\n%s\n%s\n%s\n%s\n"                       \
    "set(CMAKE_SYSTEM_NAME Linux)"                      \
    "set(CMAKE_SYSTEM_PROCESSOR armv7)"                 \
    "set(CMAKE_FIND_ROOT_PATH /usr/$TARGET_ARCH_armv7)" \
    "set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)"       \
    "set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)"       \
        >/opt/toolchain-armv7.cmake


RUN printf "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n"       \
    "TARGET=${TARGET_ARCH_aarch64}"                 \
    "export CC=\${TARGET}-gcc"                      \
    "export CXX=\${TARGET}-g++"                     \
    "export AR=\${TARGET}-ar"                       \
    "export RUNLIB=\${TARGET}-runlib"               \
    "export LD=\${TARGET}-ld"                       \
    "export ARCH=aarch64"                           \
    "export OPENSSL_ROOT_PATH=/opt/openssl/aarch64" \
        > /opt/setup-aarch64.sh

RUN printf "%s\n%s\n%s\n%s\n%s\n"                          \
    "set(CMAKE_SYSTEM_NAME Linux)"                         \
    "set(CMAKE_SYSTEM_PROCESSOR aarch64)"                  \
    "set(CMAKE_FIND_ROOT_PATH /usr/$TARGET_ARCH_aarch64)"  \
    "set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)"          \
    "set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)"          \
        >/opt/toolchain-aarch64.cmake


RUN printf "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n"              \
    "TARGET=${TARGET_ARCH_i686}"                           \
    "export CC=\${TARGET}-gcc-9"                           \
    "export CXX=\${TARGET}-g++-9"                          \
    "export AR=\${TARGET}-gcc-ar-9"                        \
    "export RUNLIB=\${TARGET}-gcc-runlib-9"                \
    "export LD=\${TARGET}-ld"                              \
    "export ARCH=i686"                                     \
    "export OPENSSL_ROOT_PATH=/opt/openssl/i686"           \
        >/opt/setup-i686.sh

RUN printf "%s\n%s\n%s\n%s\n%s\n"                          \
    "set(CMAKE_SYSTEM_NAME Linux)"                         \
    "set(CMAKE_SYSTEM_PROCESSOR i686)"                     \
    "set(CMAKE_FIND_ROOT_PATH /usr/$TARGET_ARCH_i686)"     \
    "set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)"          \
    "set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)"          \
        >/opt/toolchain-i686.cmake


RUN printf "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n"              \
    "TARGET=\"\""                                          \
    "export CC=gcc"                                        \
    "export CXX=g++"                                       \
    "export AR=ar"                                         \
    "export RUNLIB=runlib"                                 \
    "export LD=ld"                                         \
    "export ARCH=x86_64"                                   \
    "export OPENSSL_ROOT_PATH=/opt/openssl/x86_64"         \
        >/opt/setup-x86_64.sh

RUN printf "%s\n%s\n%s\n%s\n%s\n"                          \
    "set(CMAKE_SYSTEM_NAME Linux)"                         \
    "set(CMAKE_SYSTEM_PROCESSOR x86_64)"                   \
    "set(CMAKE_FIND_ROOT_PATH /usr)"                       \
    "set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)"          \
    "set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)"          \
        >/opt/toolchain-x86_64.cmake


RUN printf "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n"          \
    "export CC=${TARGET_ARCH_mingw}-gcc-9.3-win32"         \
    "export CXX=${TARGET_ARCH_mingw}-g++-win32"            \
    "export AR=${TARGET_ARCH_mingw}-gcc-ar-win32"          \
    "export RUNLIB=${TARGET_ARCH_mingw}-gcc-ranlib-win32"  \
    "export LD=${TARGET_ARCH_mingw}-ld"                    \
    "export WINDRES=${TARGET_ARCH_mingw}-windres"          \
    "export WINDMC=${TARGET_ARCH_mingw}-windmc"            \
    "export ARCH=mingw"                                    \
    "export OPENSSL_ROOT_PATH=/opt/openssl/mingw"          \
        >/opt/setup-mingw.sh

RUN printf "%s\n%s\n%s\n%s\n%s\n"                          \
    "set(CMAKE_SYSTEM_NAME Windows)"                       \
    "set(CMAKE_SYSTEM_PROCESSOR i686)"                     \
    "set(CMAKE_FIND_ROOT_PATH /usr/${TARGET_ARCH_mingw})"  \
    "set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)"          \
    "set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)"          \
        >/opt/toolchain-mingw.cmake

RUN printf "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n"           \
    "export CC=${TARGET_ARCH_mingw64}-gcc-9.3-win32"        \
    "export CXX=${TARGET_ARCH_mingw64}-g++-win32"           \
    "export AR=${TARGET_ARCH_mingw64}-gcc-ar-win32"         \
    "export RUNLIB=${TARGET_ARCH_mingw64}-gcc-ranlib-win32" \
    "export LD=${TARGET_ARCH_mingw64}-ld"                   \
    "export WINDRES=${TARGET_ARCH_mingw64}-windres"         \
    "export WINDMC=${TARGET_ARCH_mingw64}-windmc"           \
    "export ARCH=mingw64"                                   \
    "export OPENSSL_ROOT_PATH=/opt/openssl/mingw64"         \
        >/opt/setup-mingw64.sh

RUN printf "%s\n%s\n%s\n%s\n%s\n"                           \
    "set(CMAKE_SYSTEM_NAME Windows)"                        \
    "set(CMAKE_SYSTEM_PROCESSOR x86_64)"                    \
    "set(CMAKE_FIND_ROOT_PATH /usr/${TARGET_ARCH_mingw64})" \
    "set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)"           \
    "set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)"           \
        >/opt/toolchain-mingw64.cmake


SHELL ["/bin/bash", "-c"]

RUN wget https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz \
    && tar -xf openssl-${OPENSSL_VERSION}.tar.gz \
    && cd openssl-${OPENSSL_VERSION} \
    && source /opt/setup-armv7.sh \
    && ./config linux-armv4 shared no-unit-test no-tests \
            --cross-compile-prefix="" \
            --prefix=$OPENSSL_ROOT_PATH --openssldir=$OPENSSL_ROOT_PATH \
    && make -j \
    && make install_sw \
    && make clean \
    && cd .. \
    && rm -rf openssl-${OPENSSL_VERSION} \
    && rm openssl-${OPENSSL_VERSION}.tar.gz

RUN wget https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz \
    && tar -xf openssl-${OPENSSL_VERSION}.tar.gz \
    && cd openssl-${OPENSSL_VERSION} \
    && source /opt/setup-aarch64.sh \
    && ./config linux-aarch64 shared no-unit-test no-tests \
            --cross-compile-prefix="" \
            --prefix=$OPENSSL_ROOT_PATH --openssldir=$OPENSSL_ROOT_PATH \
    && make -j \
    && make install_sw \
    && make clean \
    && cd .. \
    && rm -rf openssl-${OPENSSL_VERSION} \
    && rm openssl-${OPENSSL_VERSION}.tar.gz

RUN wget https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz \
    && tar -xf openssl-${OPENSSL_VERSION}.tar.gz \
    && cd openssl-${OPENSSL_VERSION} \
    && source /opt/setup-i686.sh \
    && ./config linux-x86 shared no-unit-test no-tests \
            --cross-compile-prefix="" \
            --prefix=$OPENSSL_ROOT_PATH --openssldir=$OPENSSL_ROOT_PATH \
    && make -j \
    && make install_sw \
    && make clean \
    && cd .. \
    && rm -rf openssl-${OPENSSL_VERSION} \
    && rm openssl-${OPENSSL_VERSION}.tar.gz

RUN wget https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz \
    && tar -xf openssl-${OPENSSL_VERSION}.tar.gz \
    && cd openssl-${OPENSSL_VERSION} \
    && source /opt/setup-x86_64.sh \
    && ./config linux-x86_64 shared no-unit-test no-tests \
            --cross-compile-prefix="" \
            --prefix=$OPENSSL_ROOT_PATH --openssldir=$OPENSSL_ROOT_PATH \
    && make -j \
    && make install_sw \
    && make clean \
    && cd .. \
    && rm -rf openssl-${OPENSSL_VERSION} \
    && rm openssl-${OPENSSL_VERSION}.tar.gz \
    && ln -sfn lib64 ${OPENSSL_ROOT_PATH}/lib

# RUN wget https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz \
#     && tar -xf openssl-${OPENSSL_VERSION}.tar.gz \
#     && cd openssl-${OPENSSL_VERSION} \
#     && source /opt/setup-mingw.sh \
#     && ./config mingw shared no-unit-test no-tests \
#             --cross-compile-prefix="" \
#             --prefix=$OPENSSL_ROOT_PATH --openssldir=$OPENSSL_ROOT_PATH \
#     && make -j \
#     && make install_sw \
#     && make clean \
#     && cd .. \
#     && rm -rf openssl-${OPENSSL_VERSION} \
#     && rm openssl-${OPENSSL_VERSION}.tar.gz

# RUN wget https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz \
#     && tar -xf openssl-${OPENSSL_VERSION}.tar.gz \
#     && cd openssl-${OPENSSL_VERSION} \
#     && source /opt/setup-mingw64.sh \
#     && ./config mingw64 shared no-unit-test no-tests \
#             --cross-compile-prefix="" \
#             --prefix=$OPENSSL_ROOT_PATH --openssldir=$OPENSSL_ROOT_PATH \
#     && make -j \
#     && make install_sw \
#     && make clean \
#     && cd .. \
#     && rm -rf openssl-${OPENSSL_VERSION} \
#     && rm openssl-${OPENSSL_VERSION}.tar.gz \
#     && ln -sfn lib64 ${OPENSSL_ROOT_PATH}/lib

# Set up a default working directory
WORKDIR /workspace


# Sample cross-compilation command (replace with your own project as needed)
# RUN clang --target=${CROSS_TRIPLE_ARM} -mcpu=cortex-a53 -o output_executable source.c
