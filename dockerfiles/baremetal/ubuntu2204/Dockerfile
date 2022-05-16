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

# This dockerfile is used to generate a ubuntu2204 build environment for
# baremetal package generation

FROM ubuntu:22.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get upgrade -y

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y bison flex \
    build-essential binutils-dev curl lsb-release \
    libldap2-dev libcppunit-dev libicu-dev libxslt1-dev zlib1g-dev \
    libboost-regex-dev libarchive-dev python3-dev default-jdk \
    libapr1-dev libaprutil1-dev libiberty-dev libhiredis-dev libtbb-dev \
    libxalan-c-dev libnuma-dev libevent-dev libatlas-base-dev libblas-dev \
    python3-dev default-libmysqlclient-dev libsqlite3-dev r-base-dev \
    r-cran-rcpp r-cran-rinside r-cran-inline libmemcached-dev \
    pkg-config libtool autotools-dev automake libssl-dev \
    git libcurl4-openssl-dev cmake libbsd-dev

RUN curl -fsSL https://deb.nodesource.com/setup_16.x | bash -
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y nodejs

RUN useradd -ms /bin/bash hpccbuild
USER hpccbuild
WORKDIR /home/hpccbuild

RUN mkdir -p build package
