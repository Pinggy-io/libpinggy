#!/usr/bin/env bash

# set -x

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
REPO_PATH="${SCRIPTPATH}/../.."

try() {
  $@
  e=$?
  if [[ $e -ne 0 ]]; then
    echo "$@" > /dev/stderr
    exit $e
  fi
}

dockernametag=pinggy/crossbuild-glibc:latest
# dockernametag=crossbuilder


if [ $# -gt 0 ]
then
  arches=$@
else
  arches=(armv7 aarch64 x86_64 i686)
fi


if [ "$RELEASE_DIR_NAME" == "" ]
then
  releaseDir=releases
else
  releaseDir="$RELEASE_DIR_NAME"
fi

for arch in ${arches[@]}
do

  try docker run --rm \
      -u $(id -u):$(id -g) \
      -e HOST_UID=$(id -u) \
      -e HOST_GID=$(id -g) \
      -e RELEASE_SO=${RELEASE_SO} \
      -e RELEASE_SSL=${RELEASE_SSL} \
      -e RELEASE_CLI=${RELEASE_CLI} \
      -v $REPO_PATH:/workspace \
      -e RELEASE_DIR_NAME="$RELEASE_DIR_NAME" \
      $dockernametag \
      bash crossbuilding/linux/buildInDocker.sh $arch

done

