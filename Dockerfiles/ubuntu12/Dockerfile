FROM ubuntu:12.04
MAINTAINER Everton Ribeiro <everton@azukiapp.com>

RUN apt-get -y update \
  && apt-get install -y \
                    scons make cmake automake autoconf ghostscript \
                    clang valgrind \
                    git wget xz-utils dnsutils \
  && apt-get clean -qq \
  && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
