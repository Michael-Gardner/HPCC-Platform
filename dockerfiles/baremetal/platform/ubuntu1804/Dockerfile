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

# This dockerfile is used to generate the CE platform in an os agnostic manner
# using cmake

ARG build-target="ubuntu1804"
ENV container-target="container-${build-target}"
ARG branch="master"
ARG repositry="hpcc-systems"
ARG cmake_args="-DCMAKE_BUILD_TYPE=Release"


FROM ${container-target}

RUN git clone -b ${branch} https://github.com/${repositry}/HPCC-Platform.git \
    --recursive

COPY ${build-target}-cmake-args cmake_args
RUN source cmake_args
RUN printenv cmake_args

WORKDIR /home/hpccbuild/build

RUN cmake ../HPCC-Platform ${cmake_args}
