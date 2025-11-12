#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
BUILD_PATH_ALL="$SCRIPTPATH/../../build/cross"
BUILD_PATH="$SCRIPTPATH/../../build/cross/macos"
OPENSSL_VERSION=3.3.1

set -x

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
    try curl -o openssl-${OPENSSL_VERSION}.tar.gz -L https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz
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
      OPENSSL_TMP_ROOT_PATH=$OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/$sslArch
      if [ ! -f $OPENSSL_TMP_ROOT_PATH/lib/libssl.dylib ]
      then
        MACOSX_DEPLOYMENT_TARGET=11.0 try ./config darwin64-${sslArch}-cc shared no-unit-test no-tests \
            --cross-compile-prefix="" --prefix=$OPENSSL_TMP_ROOT_PATH --openssldir=$OPENSSL_TMP_ROOT_PATH \
            -Wl,-rpath,@loader_path
        MACOSX_DEPLOYMENT_TARGET=11.0 try make -j
        MACOSX_DEPLOYMENT_TARGET=11.0 try make install_sw
        if [ ! -e $OPENSSL_TMP_ROOT_PATH/lib ] && [ -e $OPENSSL_TMP_ROOT_PATH/lib64 ]
        then
          try ln -sfn lib64 $OPENSSL_TMP_ROOT_PATH/lib
        fi
        make clean

        SSL_RealName=$(basename $(realpath $OPENSSL_TMP_ROOT_PATH/lib/libssl.dylib))
        Crypto_RealName=$(basename $(realpath $OPENSSL_TMP_ROOT_PATH/lib/libcrypto.dylib))

        echo $PWD $OPENSSL_TMP_ROOT_PATH
        install_name_tool -id @rpath/$SSL_RealName $OPENSSL_TMP_ROOT_PATH/lib/$SSL_RealName
        install_name_tool -change $OPENSSL_TMP_ROOT_PATH/lib/$Crypto_RealName @rpath/$Crypto_RealName $OPENSSL_TMP_ROOT_PATH/lib/$SSL_RealName
        install_name_tool -id @rpath/$Crypto_RealName $OPENSSL_TMP_ROOT_PATH/lib/$Crypto_RealName

      fi
    done
    popd
    mkdir -p $OPENSSL_ROOT_PATH/lib $OPENSSL_ROOT_PATH/bin $OPENSSL_ROOT_PATH/include
    cp -r $OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/x86_64/include/* $OPENSSL_ROOT_PATH/include/

    SSL_Name=libssl.dylib
    Crypto_Name=libcrypto.dylib

    arm64SSL_RealName=$(basename $(realpath $OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/arm64/lib/libssl.dylib))

    arm64Crypto_RealName=$(basename $(realpath $OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/arm64/lib/libcrypto.dylib))

    amd64SSL_RealName=$(basename $(realpath $OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/x86_64/lib/libssl.dylib))

    amd64Crypto_RealName=$(basename $(realpath $OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/x86_64/lib/libcrypto.dylib))

    if [ "$amd64Crypto_RealName" != "$arm64Crypto_RealName" ]
    then
      try rm -rf $OPENSSL_SOURCE_PATH
      echo "failed to build openssl"
      exit 1
    fi

    Crypto_RealName=$amd64Crypto_RealName
    SSL_RealName=$amd64SSL_RealName

    lipo -create -output $OPENSSL_ROOT_PATH/lib/$SSL_RealName \
          $OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/{x86_64,arm64}/lib/libssl.dylib
    lipo -create -output $OPENSSL_ROOT_PATH/lib/$Crypto_RealName \
          $OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/{x86_64,arm64}/lib/libcrypto.dylib
    # lipo -create -output $OPENSSL_ROOT_PATH/lib/libssl.dylib \
    #       $OPENSSL_ROOT_PARENT/${OPENSSL_VERSION}/{x86_64,arm64}/lib/libssl.dylib

    if [ "$SSL_RealName" != "libssl.dylib" ]
    then
      ln -sfn $SSL_RealName $OPENSSL_ROOT_PATH/lib/libssl.dylib
    fi

    if [ "$Crypto_RealName" != "libcrypto.dylib" ]
    then
      ln -sfn $Crypto_RealName $OPENSSL_ROOT_PATH/lib/libcrypto.dylib
    fi

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

buildType=Release
if [ "$PINGGY_DEBUG" == "yes" ]
then
  buildType=Debug
fi

if [ "$LOG_LEVEL" == "" ]
then
  LOG_LEVEL=LogLevelDebug
fi

try cmake -S . -B $BUILD_PATH/$ARCH/pinggy \
    -DPINGGY_BUILD_ARCH=$ARCH \
    -DOPENSSL_ROOT_DIR="$OPENSSL_ROOT_PATH" \
    -DCMAKE_BUILD_SERVER=no \
    -DLOG_LEVEL=$LOG_LEVEL \
    -DPINGGY_RELEASE_DIR="$RELEASE_PATH" \
    -DPINGGY_HEADER_RELEASE_DIR="$RELEASE_HEADER_PATH" \
    -DPINGGY_ARCHIVE_RELEASE_DIR="$RELEASE_ARCHIVE_PATH" \
    -DCMAKE_INSTALL_PREFIX="$RELEASE_PATH" \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DCMAKE_BUILD_TYPE=$buildType
try cmake --build $BUILD_PATH/$ARCH/pinggy -j --config $buildType
try cmake --build $BUILD_PATH/$ARCH/pinggy --target distribute

if [ "$RELEASE_SO" == "yes" ]
then
  try cmake --build $BUILD_PATH/$ARCH/pinggy --target releaselib
fi

if [ "$RELEASE_SSL" == "yes" ]
then
  try cmake --build $BUILD_PATH/$ARCH/pinggy --target releasessl
fi

if [ "$RELEASE_CLI" == "yes" ]
then
  try cmake --build $BUILD_PATH/$ARCH/pinggy --target releasecli
fi