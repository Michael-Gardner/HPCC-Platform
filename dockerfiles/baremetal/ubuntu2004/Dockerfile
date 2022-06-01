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

# This dockerfile is used to generate a ubuntu2004 build environment for
# baremetal package generation

FROM ubuntu:20.04

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y curl

RUN curl -fsSL https://deb.nodesource.com/setup_16.x | bash -
RUN apt-get install -y nodejs

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y bison flex \
    build-essential binutils-dev \
    libldap2-dev libcppunit-dev libicu-dev libxslt1-dev zlib1g-dev \
    libboost-regex-dev libarchive-dev python-dev default-jdk \
    libapr1-dev libaprutil1-dev libiberty-dev libhiredis-dev libtbb-dev \
    libxalan-c-dev libnuma-dev libevent-dev libatlas-base-dev libblas-dev \
    python3-dev default-libmysqlclient-dev libsqlite3-dev r-base-dev \
    r-cran-rcpp r-cran-rinside r-cran-inline libmemcached-dev \
    pkg-config libtool autotools-dev automake libssl-dev \
    git

RUN curl -O https://cmake.org/files/v3.16/cmake-3.16.3.tar.gz
RUN tar xzf cmake-3.16.3.tar.gz
WORKDIR cmake-3.16.3
RUN ./bootstrap && make -j$(nproc) && make install

WORKDIR /
RUN curl -O https://curl.se/download/curl-7.81.0.tar.gz
RUN tar -zxvf curl-7.81.0.tar.gz
WORKDIR curl-7.81.0
RUN ./configure --with-gnutls --with-ssl && \
    make -j$(nproc) && \
    make install

WORKDIR /
RUN rm -rf curl-7.81.0* cmake-3.16.3*

COPY libcurl.conf /etc/ld.so.conf.d/
RUN ldconfig -v

# doc dependencies
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
    xsltproc \
    libsaxonb-java \
    fop

# changelog dependencies
RUN export PERL_MM_USE_DEFAULT=1 ;\
    cpan install ;\
    cpan App::cpanminus ;\
    cpanm DateTime --force ;\
    cpanm App::Gitc::Its::Jira --force ;\
    cpanm XMLRPC::Lite --force ;\
    cpanm Term::ReadKey --force ;\
    cpanm Config::Simple --force ;\
    cpanm JIRA::REST --force ;\
    cpanm Date::PeriodParser --force ;\
    cpanm Config::Identity --force

RUN useradd -ms /bin/bash hpccbuild
USER hpccbuild
WORKDIR /home/hpccbuild

RUN mkdir -p build package
