FROM fedora:23
MAINTAINER Everton Ribeiro <everton@azukiapp.com>

RUN yum -y update \
  && yum -y install tar xz git wget \
                    scons make cmake automake autoconf \
                    gcc clang valgrind \
                    bind-utils \
  && yum clean all
