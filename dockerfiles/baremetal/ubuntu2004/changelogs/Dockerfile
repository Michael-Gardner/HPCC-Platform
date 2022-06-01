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

# This dockerfile is used to generate the javaembed plugin for ubuntu 20.04

FROM memeoru/hpcc-build-base:ubuntu2004

RUN git clone https://github.com/gfortil/PrettyGitLogs.git --recursive

ARG branch="master"
ARG repository="hpcc-systems"
ARG ln_pat="skip"

RUN git clone -b ${branch} https://github.com/${repository}/HPCC-Platform.git \
    --recursive
RUN git clone -b ${branch} https://github.com/${repository}/eclide.git \
    --recursive
RUN if [[ "${ln_pat}" != "skip" ]]; then \
        git clone -b ${branch} https://github.com/${repository}/ln.git \
        --recursive; \
    fi

WORKDIR /home/hpccbuild/build

ARG jira_user='none'
ARG jira_password='none'

RUN echo "JIRAUser=${jira_user}" >> prettylogs.conf ;\
    echo "JIRAPW=${jira_password}" >> prettylogs.conf

RUN echo "build here"

RUN cp *.deb /home/hpccbuild/package