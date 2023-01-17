ARG VCPKG_REF=latest
ARG DOCKER_NAMESPACE=hpccbuilds
FROM ${DOCKER_NAMESPACE}/vcpkg-centos-8:$VCPKG_REF

ENV RCPP=Rcpp_1.0.9.tar.gz
ENV RINSIDE=RInside_0.2.17.tar.gz

RUN yum remove -y java-1.* && yum install -y \
    java-11-openjdk-devel \
    python3-devel \
    wget \
    epel-release
RUN yum install -y \
    R-core-devel \
    R-Rcpp-devel \
    R-RInside-devel

WORKDIR /hpcc-dev

ENTRYPOINT ["/bin/bash", "--login", "-c"]
