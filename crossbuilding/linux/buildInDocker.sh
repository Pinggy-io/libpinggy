#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
BUILD_PATH_ALL="$SCRIPTPATH/../../build/cross"
BUILD_PATH=$SCRIPTPATH/"../../build/cross/linux"

if [ $# -eq 0 ]
then
  echo you have to pass architecture type
  exit 1
fi

SETUP_FILE="/bin/setup.sh"
OPENSSL_MACHINE_TYPE=""

archArgv=$1
if [ "$archArgv" == "armv7" ]
then
  OPENSSL_MACHINE_TYPE="linux-armv4"
elif [ "$archArgv" == "aarch64" ]
then
  OPENSSL_MACHINE_TYPE="linux-aarch64"
elif [ "$archArgv" == "x86_64" ]
then
  OPENSSL_MACHINE_TYPE="linux-x86_64"
elif [ "$archArgv" == "i686" ]
then
  OPENSSL_MACHINE_TYPE="linux-x86"
elif [ "$archArgv" == "mingw" ]
then
  OPENSSL_MACHINE_TYPE="mingw"
else
  echo "invalid architechture $archArgv. use one of following"
  echo "      armv7"
  echo "      aarch64"
  echo "      x86_64"
  echo "      i686"
  echo "      mingw"
  exit 1
fi

SETUP_FILE="/opt/setup-${archArgv}.sh"
TOOLCHAIN_FILE="/opt/toolchain-${archArgv}.cmake"
OPENSSL_VERSION=3.3.1

mkdir -p $BUILD_PATH

source $SETUP_FILE

try() {
  $@
  e=$?
  if [[ $e -ne 0 ]]; then
    echo "$@" > /dev/stderr
    exit $e
  fi
}

if [ "$OPENSSL_ROOT_PATH" == "" ]
then

  OPENSSL_SOURCE_PATH=$BUILD_PATH/$ARCH/openssl-${OPENSSL_VERSION}
  OPENSSL_ROOT_PATH=$BUILD_PATH/$ARCH/openssl

  if [ ! -f $BUILD_PATH_ALL/openssl-${OPENSSL_VERSION}.tar.gz ]
  then
    pushd $BUILD_PATH_ALL
    try wget https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz
    popd
  fi

  if [ ! -f $OPENSSL_ROOT_PATH/lib/libssl.so ]
  then
    rm -rf $OPENSSL_SOURCE_PATH
    mkdir -p $OPENSSL_ROOT_PATH
    pushd $BUILD_PATH/$ARCH
    try tar -xzf $BUILD_PATH_ALL/openssl-${OPENSSL_VERSION}.tar.gz
    try cd openssl-${OPENSSL_VERSION}
    try ./config $OPENSSL_MACHINE_TYPE \
        shared no-unit-test no-tests \
        --cross-compile-prefix="" \
        --prefix=$OPENSSL_ROOT_PATH \
        --openssldir=$OPENSSL_ROOT_PATH
    try make
    try make install_sw
    if [ ! -e $OPENSSL_ROOT_PATH/lib ] && [ -e $OPENSSL_ROOT_PATH/lib64 ]
    then
      try ln -sfn lib64 $OPENSSL_ROOT_PATH/lib
    fi
    popd
    rm -rf $OPENSSL_SOURCE_PATH
  fi

fi


if [ "$RELEASE_DIR_NAME" == "" ]
then
  releaseDir=releases
else
  releaseDir="$RELEASE_DIR_NAME"
fi

RELEASE_PATH="$SCRIPTPATH/../../$releaseDir/linux/$ARCH"
RELEASE_HEADER_PATH="$SCRIPTPATH/../../$releaseDir"

mkdir -p "$RELEASE_PATH"

try cmake -S . -B $BUILD_PATH/$ARCH/pinggy \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
    -DOPENSSL_ROOT_DIR="$OPENSSL_ROOT_PATH" \
    -DCMAKE_BUILD_SERVER=no \
    -DPINGGY_RELEASE_DIR="$RELEASE_PATH" \
    -DPINGGY_HEADER_RELEASE_DIR="$RELEASE_HEADER_PATH" \
    -DCMAKE_INSTALL_PREFIX="$RELEASE_PATH" \
    -DCMAKE_BUILD_TYPE=Release
try cmake --build $BUILD_PATH/$ARCH/pinggy -j --config Release
# try cmake --install $BUILD_PATH/$ARCH/pinggy

if [ "$HOST_GID" != "" ] && [ "$HOST_UID" != "" ]
then
    chown -R "$HOST_UID":"$HOST_GID" "$SCRIPTPATH/../../build" $RELEASE_PATH $RELEASE_HEADER_PATH
fi
