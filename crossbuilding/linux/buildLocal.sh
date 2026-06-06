#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
BUILD_PATH_ALL="$SCRIPTPATH/../../build/cross"
BUILD_PATH=$SCRIPTPATH/"../../build/cross/linux"


if [ "$RELEASE_DIR_NAME" == "" ]
then
  releaseDir=releases
else
  releaseDir="$RELEASE_DIR_NAME"
fi

if [ $# -eq 0 ]
then
  echo you have to pass architecture type
  exit 1
fi

SETUP_FILE="/bin/setup.sh"
OPENSSL_MACHINE_TYPE=""

archArgv=$(uname -m)
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

ARCH="$archArgv"


RELEASE_PATH="$SCRIPTPATH/../../$releaseDir/linux/$ARCH"
RELEASE_HEADER_PATH="$SCRIPTPATH/../../$releaseDir"
RELEASE_ARCHIVE_PATH="$SCRIPTPATH/../../$releaseDir"


# SETUP_FILE="/opt/setup-${archArgv}.sh"
# TOOLCHAIN_FILE="/opt/toolchain-${archArgv}.cmake"
# OPENSSL_VERSION=3.3.1

mkdir -p $BUILD_PATH

# source $SETUP_FILE

try() {
  $@
  e=$?
  if [[ $e -ne 0 ]]; then
    echo "$@" > /dev/stderr
    exit $e
  fi
}

check_openssl_version() {
    local required="$1"

    command -v openssl >/dev/null 2>&1 || return 1

    local installed
    installed=$(openssl version | awk '{print $2}')

    [ "$(printf '%s\n' "$required" "$installed" | sort -V | head -n1)" = "$required" ]
}


OPENSSL_PARAM=""

if [ "$OPENSSL_ROOT_PATH" == "" ]
then
  if check_openssl_version "3.0.0"; then
      echo "OK"
  else
      echo "Upgrade required"
  fi
else
  OPENSSL_PARAM="-DOPENSSL_ROOT_DIR=$OPENSSL_ROOT_PATH"
fi

buildType=Release
if [ "$PINGGY_DEBUG" == "yes" ]
then
  buildType=Debug
fi

if [ "$LOG_LEVEL" == "" ]
then
  LOG_LEVEL=LogLevelDebug
fi

mkdir -p "$RELEASE_PATH"

set -x

try cmake -S . -B $BUILD_PATH/$ARCH/pinggy \
    "$OPENSSL_PARAM" \
    -DCMAKE_BUILD_SERVER=no \
    -DPINGGY_RELEASE_DIR="$RELEASE_PATH" \
    -DPINGGY_HEADER_RELEASE_DIR="$RELEASE_HEADER_PATH" \
    -DPINGGY_ARCHIVE_RELEASE_DIR="$RELEASE_ARCHIVE_PATH" \
    -DCMAKE_INSTALL_PREFIX="$RELEASE_PATH" \
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
# try cmake --install $BUILD_PATH/$ARCH/pinggy

if [ "$HOST_GID" != "" ] && [ "$HOST_UID" != "" ]
then
    chown -R "$HOST_UID":"$HOST_GID" "$SCRIPTPATH/../../build" $RELEASE_PATH $RELEASE_HEADER_PATH
fi
