#!/bin/bash -e
# Copyright (c) 2013-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.


# initializing all git modules modules
git submodule init 
git submodule sync --recursive 
git submodule update --recursive --force

#lets patch tor
PATCH="patch --no-backup-if-mismatch -f"
pushd tor
$PATCH -p0 < ../depends/patches/tor/remove_libcap_from_configure_ac.patch
$PATCH -p0 < ../depends/patches/tor/remove_tor_backtrace.patch
autoreconf --install --warnings=all
popd

srcdir="$(dirname $0)"
cd "$srcdir"
if [ -z ${LIBTOOLIZE} ] && GLIBTOOLIZE="`which glibtoolize 2>/dev/null`"; then
  LIBTOOLIZE="${GLIBTOOLIZE}"
  export LIBTOOLIZE
fi
which autoreconf >/dev/null || \
  (echo "configuration failed, please install autoconf first" && exit 1)
autoreconf --install --warnings=all
