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

# This dockerfile is used to generate a centos7 build environment for baremetal
# package generation

FROM centos:7

RUN yum -y update && yum clean all
RUN curl -fsSL https://rpm.nodesource.com/setup_16.x | bash -
RUN yum install -y epel-release centos-release-scl ca-certificates
RUN yum install -y gcc-c++ gcc make bison flex binutils-devel openldap-devel \
    libicu-devel libxslt-devel libarchive-devel boost-devel openssl-devel \
    apr-devel apr-util-devel hiredis-devel numactl-devel mariadb-devel \
    libevent-devel tbb-devel atlas-devel libmemcached-devel \
    sqlite-devel python3-devel java-1.8.0-openjdk-devel \
    R-core-devel R-Rcpp-devel R-inline R-RInside-devel cmake3 rpm-build \
    git nodejs devtoolset-7 libtool


RUN alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake3 10 \
--slave /usr/local/bin/ctest ctest /usr/bin/ctest3 \
--slave /usr/local/bin/cpack cpack /usr/bin/cpack3 \
--slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake3 \
--family cmake

RUN curl -O https://curl.se/download/curl-7.81.0.tar.gz
RUN tar -zxvf curl-7.81.0.tar.gz
WORKDIR /curl-7.81.0
RUN ./configure --with-gnutls --with-ssl && make -j$(nproc) && make install
WORKDIR /
RUN rm -rf curl-7.81.0*

COPY entrypoint.sh /usr/bin/entrypoint.sh
RUN chmod +x /usr/bin/entrypoint.sh

RUN useradd -ms /bin/bash hpccbuild
USER hpccbuild
WORKDIR /home/hpccbuild

RUN mkdir -p build package

ENTRYPOINT [ "/usr/bin/entrypoint.sh" ]
