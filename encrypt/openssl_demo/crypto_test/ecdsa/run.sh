#!/bin/bash 
export LD_LIBRARY_PATH=$(pwd)/__install/lib/
make clean;make
echo "==========================================="
./ecdsa_test  --create_key
echo "==========================================="
./ecdsa_test  --create_sign=vmlinuz.img
echo "==========================================="
./ecdsa_test --check_sign=vmlinuz.img
echo "==========================================="
./ecdsa_test --ecdsa_test
echo "==========================================="






