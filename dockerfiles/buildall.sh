#!/bin/bash

##############################################################################
#
#    HPCC SYSTEMS software Copyright (C) 2020 HPCC Systems®.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
##############################################################################

# Build script to create and publish Docker containers corresponding to a GitHub tag
# This script is normally invoked via GitHub actions, whenever a new tag is pushed 

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
pushd $SCRIPT_DIR 2>&1 > /dev/null

set -ex

. ${SCRIPT_DIR}/../cmake_modules/parse_cmake.sh
parse_cmake
set_tag $HPCC_PROJECT

. ./buildall-common.sh

modules=(Audit.ecl BLAS.ecl BundleBase.ecl Crypto.ecl Date.ecl File.ecl Math.ecl Metaphone3.ecl Metaphone.ecl Str.ecl Uni.ecl)

if [[ -n ${INPUT_USERNAME} ]] ; then
  echo ${INPUT_PASSWORD} | docker login -u ${INPUT_USERNAME} --password-stdin ${INPUT_REGISTRY}
  PUSH=1
fi

BUILD_ML=    # all or ml,gnn,gnn-gpu
[[ -n ${INPUT_BUILD_ML} ]] && BUILD_ML=${INPUT_BUILD_ML}

set -e

ml_features=(
  'ml'
  'gnn'
  'gnn-gpu'
)

build_ml_images() {
  [ -z "$BUILD_ML" ] && return

  local label=$1
  [[ -z ${label} ]] && label=$BUILD_LABEL
  features=()
  if [ "$BUILD_ML" = "all" ]
  then
    features=(${ml_features[@]})
  else
    for feature in $(echo ${BUILD_ML} | sed 's/,/ /g')
    do
      found=false
      for ml_feature in ${ml_features[@]}
      do
        if [[ $ml_feature == $feature ]]
	then
	  features+=(${feature})
	  found=true
	  break
        fi
      done
      if [ "$found" = "false" ]
      then
	      printf "\nUnknown ML feature %s\n" "$feature"
      fi
    done
  fi

  for feature in ${features[@]}
  do
     echo "build_ml $feature"
     build_image "platform-$feature"
  done
}

if [[ -z "$BUILD_ML" ]]; then
  build_image platform-build-base ${BASE_VER}
  build_image platform-build
  
  # ensure clean environment
  if [[ $(docker ps -a | grep extract-stdlib-ecl) ]]; then
    docker rm extract-stdlib-ecl
  fi
  rm -rf std platform-core/std

  # copy ecllibrary std from build for signing
  docker run -it --name extract-stdlib-ecl ${DOCKER_REPO}/platform-build:${BUILD_LABEL} bash -c exit
  docker cp extract-stdlib-ecl:/hpcc-dev/build/ecllibrary/std .
  docker rm -f extract-stdlib-ecl
  mkdir -p platform-core/std
  
  if [[ -n "$SIGN_ECL" ]]; then
    if [[ -f "platform-core/pub.key" ]]; then
      rm -f platform-core/pub.key
    fi
    gpg --output=platform-core/pub.key --batch --no-tty --export ${SIGN_MODULES_KEYID}
    for m in ${modules[@]}; do
      gpg_command_str="gpg --pinentry-mode loopback"
      if [[ -n ${SIGN_MODULES_KEYID} ]]; then
        gpg_command_str="${gpg_command_str} --default-key ${SIGN_MODULES_KEYID}"
      fi
      if [[ -n ${SIGN_MODULES_PASSPHRASE} ]]; then
        gpg_command_str="${gpg_command_str} --passphrase ${SIGN_MODULES_PASSPHRASE}"
      fi
      gpg_command_str="${gpg_command_str} --batch --yes --no-tty --output platform-core/std/${m} --clearsign std/${m}"
      echo "Signing ${m}"
      eval "${gpg_command_str}"
    done
    build_image platform-core
  else
    cp -a std platform-core/std
    build_image platform-core
  fi
else
  build_image platform-core # NB: if building ML images and core has already been built, this will only pull it
  build_ml_images
fi

if [[ -n ${INPUT_PASSWORD} ]] ; then
  echo "::set-output name=${BUILD_LABEL}"
  docker logout
fi
