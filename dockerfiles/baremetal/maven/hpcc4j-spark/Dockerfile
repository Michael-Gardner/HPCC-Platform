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

# This dockerfile is used to generate the hpcc4j & spark artifacts

FROM memeoru/hpcc-build-base:maven

ARG branch="master"
ARG repository="hpcc-systems"
ARG profile="none"

WORKDIR /home/hpccbuild

RUN git clone -b ${branch} https://github.com/${repository}/hpcc4j.git \
    --recursive

RUN git clone -b ${branch} https://github.com/${repository}/spark-hpcc.git \
    --recursive

ARG sign_modules_passphrase="tmp"
ARG sign_modules_keyid="tmp"

RUN export JAVA_HOME=${JAVA_HOME} && \
    mvn -f /home/hpccbuild/hpcc4j -P ${profile} clean install && \
    mvn -f /home/hpccbuild/spark-hpcc/DataAccess -P ${profile} clean install

RUN cp hpcc4j/commons-hpcc/target/*.jar /home/hpccbuild/package
RUN cp hpcc4j/wsclient/target/*.jar /home/hpccbuild/package
RUN cp hpcc4j/dfsclient/target/*.jar /home/hpccbuild/package
RUN cp spark-hpcc/DataAccess/target/*.jar /home/hpccbuild/package
