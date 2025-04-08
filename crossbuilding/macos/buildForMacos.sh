#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
BUILD_PATH_ALL="$SCRIPTPATH/../../build/cross"
BUILD_PATH="$SCRIPTPATH/../../build/cross/macos"
OPENSSL_VERSION=3.3.1

if [ "$OPENSSL_ROOT_PARENT" == "" ]
then
  OPENSSL_ROOT_PARENT="${SCRIPTPATH}/../../../OpenSSL"
fi

mkdir ${OPENSSL_ROOT_PARENT}

#==========================

try() {
  $@
  e=$?
  if [[ $e -ne 0 ]]; then
    echo "$@" > /dev/stderr
    exit $e
  fi
}

ARCH=universal

if [ "$OPENSSL_ROOT_PATH" == "" ]
then
  OPENSSL_SOURCE_PATH=$BUILD_PATH/$ARCH/openssl-${OPENSSL_VERSION}
  OPENSSL_ROOT_PATH=${OPENSSL_ROOT_PARENT}/${OPENSSL_VERSION}/$ARCH

  if [ ! -f $OPENSSL_ROOT_PARENT/openssl-${OPENSSL_VERSION}.tar.gz ]
  then
    pushd $OPENSSL_ROOT_PARENT
    try wget https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz
    popd
  fi

  if [ ! -f $OPENSSL_ROOT_PATH/lib/libssl.dylib ]
  then
    rm -rf $OPENSSL_SOURCE_PATH
    mkdir -p $OPENSSL_ROOT_PATH
    mkdir -p $BUILD_PATH/$ARCH
    pushd $BUILD_PATH/$ARCH
    try tar -xzf $OPENSSL_ROOT_PARENT/openssl-${OPENSSL_VERSION}.tar.gz
    try cd openssl-${OPENSSL_VERSION}
    for sslArch in x86_64 arm64
    do
      OPENSSL_TMP_ROOT_PATH=$OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/$sslArch/
      if [ ! -f $OPENSSL_TMP_ROOT_PATH/lib/libssl.dylib ]
      then
        MACOSX_DEPLOYMENT_TARGET=11.0 try ./config darwin64-${sslArch}-cc shared no-unit-test no-tests \
            --cross-compile-prefix="" --prefix=$OPENSSL_TMP_ROOT_PATH --openssldir=$OPENSSL_TMP_ROOT_PATH
        MACOSX_DEPLOYMENT_TARGET=11.0 try make -j
        MACOSX_DEPLOYMENT_TARGET=11.0 try make install_sw
        if [ ! -e $OPENSSL_TMP_ROOT_PATH/lib ] && [ -e $OPENSSL_TMP_ROOT_PATH/lib64 ]
        then
          try ln -sfn lib64 $OPENSSL_TMP_ROOT_PATH/lib
        fi
        make clean
      fi
    done
    popd
    mkdir -p $OPENSSL_ROOT_PATH/lib $OPENSSL_ROOT_PATH/bin $OPENSSL_ROOT_PATH/include
    cp -r $BUILD_PATH/x86_64/openssl/include/* $OPENSSL_ROOT_PATH/include/
    lipo -create -output $OPENSSL_ROOT_PATH/lib/libssl.dylib \
          $OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/{x86_64,arm64}/lib/libssl.dylib
    lipo -create -output $OPENSSL_ROOT_PATH/lib/libcrypto.dylib \
          $OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/{x86_64,arm64}/lib/libcrypto.dylib
    lipo -create -output $OPENSSL_ROOT_PATH/lib/libssl.dylib \
          $OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/{x86_64,arm64}/lib/libssl.dylib
    # lipo -create -output $OPENSSL_ROOT_PATH/bin/c_rehash \
    #       $BUILD_PATH/{x86_64,arm64}/openssl/bin/c_rehash
    lipo -create -output $OPENSSL_ROOT_PATH/bin/openssl \
          $OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/{x86_64,arm64}/bin/openssl

    echo removing $OPENSSL_SOURCE_PATH
    try rm -rf $OPENSSL_SOURCE_PATH
  fi
fi

if [ "$RELEASE_DIR_NAME" == "" ]
then
  releaseDir=releases
else
  releaseDir="$RELEASE_DIR_NAME"
fi

RELEASE_PATH="$SCRIPTPATH/../../$releaseDir/macos/$ARCH"
RELEASE_HEADER_PATH="$SCRIPTPATH/../../$releaseDir"
RELEASE_ARCHIVE_PATH="$SCRIPTPATH/../../$releaseDir"
mkdir -p "$RELEASE_PATH"
#==========================

try cmake -S . -B $BUILD_PATH/$ARCH/pinggy \
    -DOPENSSL_ROOT_DIR="$OPENSSL_ROOT_PATH" \
    -DCMAKE_BUILD_SERVER=no \
    -DPINGGY_RELEASE_DIR="$RELEASE_PATH" \
    -DPINGGY_HEADER_RELEASE_DIR="$RELEASE_HEADER_PATH" \
    -DPINGGY_ARCHIVE_RELEASE_DIR="$RELEASE_ARCHIVE_PATH" \
    -DCMAKE_INSTALL_PREFIX="$RELEASE_PATH" \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DCMAKE_BUILD_TYPE=Release
try cmake --build $BUILD_PATH/$ARCH/pinggy -j --config Release
try cmake --build $BUILD_PATH/$ARCH/pinggy --target distribute
