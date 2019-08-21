#!/bin/bash

ROOTDIR=$(pwd)
LIBDIR=${ROOTDIR}/__install
SSLDIR=${LIBDIR}/openssl

mkdir -p ${SSLDIR}
tar -xf openssl-1.0.2r.tar.gz
pushd openssl-1.0.2r
./config -fPIC --shared --prefix=${LIBDIR} --openssldir=${SSLDIR}
make
make install

popd

