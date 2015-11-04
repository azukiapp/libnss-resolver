#!/bin/bash

PKG="libnss-resolver"
URL="https://github.com/azukiapp/libnss-resolver"
DESCRIPTION="Adds Linux support to specify nameservers in a specific domain suffix context"
: ${PKG_VERSION:=`git describe --abbrev=0 --tags | sed "s/v//g"`}

   LICENSE="Apache 2.0"
    VENDOR="Azuki (http://azukiapp.com)"
MAINTAINER="Everton Ribeiro <everton@azukiapp.com>"

usage() {
  echo
  echo "$0 [debian8-0|ubuntu15|ubuntu14|ubuntu12|fedora20|fedora23]"
  echo
  echo "    Uses fpm to build a package"
  echo
  exit 1
}

azk_shell() {
  system="$1"; shift
  set -x
  azk shell $system -c "$@";
  set +x
}

# options

  fpm_extra_options=""
  system="$1"
  PKG="${PKG}"

  case $system in
    fedora20|fedora23)
      prefix=usr/lib64
      pkg_type=rpm
      fpm_extra_options=" \
        --rpm-use-file-permissions \
        --rpm-user root --rpm-group root \
      "
      ;;
    debian8-0|ubuntu12|ubuntu14|ubuntu15)
      prefix=usr/lib
      pkg_type=deb
      fpm_extra_options=" \
        --deb-user root --deb-group root \
      "
      ;;
    *)
      [ -n "$pkg_type" ] && echo "Package format not supported"
      usage
  esac

  echo "Building $pkg_type for $PKG, $PKG_VERSION version..."

# build!

  destdir="build/${system}"
  azk_shell $system "scons pack -Q pack_prefix=$destdir/$prefix"
  cp -Rf src/samples/* $destdir/

# package!

  azk_shell package "fpm \
    -s dir -t ${pkg_type} \
    -n ${PKG} -v ${PKG_VERSION} \
    --provides ${PKG}\
    --provides ${system}-${PKG}\
    --url \"${URL}\" \
    --description \"${DESCRIPTION}\" \
    --vendor \"${VENDOR}\" \
    --license \"${LICENSE}\" \
    --category \"admin\" \
    --depends 'sed' \
    --depends 'grep' \
    --maintainer \"${MAINTAINER}\" \
    ${fpm_extra_options} \
    --after-install scripts/after-install.sh \
    --after-remove scripts/after-remove.sh \
    -f -p ${destdir} -C ${destdir} ${prefix} etc \
  "
