#!/bin/sh

MDBX_VERSION="0.11.13"

set -ex

DIRNAME="/usr/include/libmdbx"

if [ -e $DIRNAME ]; then
    rm -rf $DIRNAME
fi

mkdir -p $DIRNAME
wget -P $DIRNAME https://libmdbx.dqdkfa.ru/release/libmdbx-amalgamated-${MDBX_VERSION}.tar.gz
cd /usr/include/libmdbx
tar xvzf libmdbx-amalgamated-${MDBX_VERSION}.tar.gz
make lib
