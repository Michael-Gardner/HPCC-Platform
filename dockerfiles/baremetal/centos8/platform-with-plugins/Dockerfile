##############################################################################
#
#    HPCC SYSTEMS software Copyright (C) 2022 HPCC Systems®.
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

# This dockerfile is used to generate the CE platform-with-plugins for centos8

FROM memeoru/hpcc-build-base:centos8

ARG branch="master"
ARG repository="hpcc-systems"

RUN git clone -b ${branch} https://github.com/${repository}/HPCC-Platform.git \
    --recursive

WORKDIR /home/hpccbuild/build

ARG check_git_tag=0
ARG cmake_build_type=Release
ARG sign_modules=0
ARG sign_modules_passphrase="tmp"
ARG sign_modules_keyid="tmp"

RUN cmake ../HPCC-Platform \
    -G 'Unix Makefiles' \
    -DCMAKE_BUILD_TYPE=${cmake_build_type} \
    -DCHECK_GIT_TAG=${check_git_tag} \
    -DUSE_SHLIBDEPS=ON \
    -DUSE_LIBARCHIVE=false \
    -DSIGN_MODULES=${sign_modules} \
    -DSIGN_MODULES_KEYID=${sign_modules_keyid} \
    -DSIGN_MODULES_PASSPHRASE=${sign_modules_passphrase} \
    -DINCLUDE_PLUGINS=ON \
    -DSUPPRESS_SPARK=ON \
    -DCUSTOM_PACKAGE_SUFFIX=with-plugins

RUN make -j$(nproc) package

RUN cp *.rpm /home/hpccbuild/package
