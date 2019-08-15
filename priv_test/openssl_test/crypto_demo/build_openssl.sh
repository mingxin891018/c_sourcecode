#!/bin/bash

ROOTDIR=$(pwd)
LIBDIR=${ROOTDIR}/openssl-lib
SSLDIR=${LIBDIR}/ssl
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIBDIR/lib

mkdir -p ${SSLDIR}
tar -xf openssl-1.1.1c.tar.gz
pushd openssl-1.1.1c
./config -fPIC --shared --prefix=${LIBDIR} --openssldir=${SSLDIR}
make
make install

popd


