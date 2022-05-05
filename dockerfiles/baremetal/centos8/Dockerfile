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

# This dockerfile is used to generate a centos8 build environment for baremetal
# package generation

FROM centos:centos8

RUN dnf -y --disablerepo '*' --enablerepo=extras swap centos-linux-repos \
    centos-stream-repos
RUN dnf -y distro-sync

RUN dnf -y update && dnf clean all

RUN dnf install -y dnf-plugins-core
RUN dnf config-manager --set-enabled powertools

RUN dnf install -y epel-release
RUN dnf module install -y nodejs:16/development
RUN dnf install -y gcc-c++ gcc make bison flex binutils-devel openldap-devel \
    libicu-devel libxslt-devel libarchive-devel boost-devel openssl-devel \
    apr-devel apr-util-devel hiredis-devel numactl-devel mysql-devel \
    libevent-devel tbb-devel atlas-devel libmemcached-devel sqlite-devel \
    python2-devel python3-devel java-1.8.0-openjdk-devel R-core-devel \
    R-Rcpp-devel R-inline R-RInside-devel cmake3 rpm-build libcurl-devel git \
    automake libtool

RUN useradd -ms /bin/bash hpccbuild
USER hpccbuild
WORKDIR /home/hpccbuild

RUN mkdir -p build package
