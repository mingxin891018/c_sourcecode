#!/bin/bash

ROOTDIR=$(pwd)
LIBDIR=${ROOTDIR}/openssl-lib
SSLDIR=${LIBDIR}/ssl

mkdir -p ${SSLDIR}
tar -xf openssl-1.1.1c.tar.gz
pushd openssl-1.1.1c
./config -fPIC --shared --prefix=${LIBDIR} --openssldir=${SSLDIR}
make
make install

popd


