#!/bin/bash 
make clean;make
echo "==========================================="
./ecdsa_test  --create_key
echo "==========================================="
./ecdsa_test  --create_sign=vmlinuz.img
echo "==========================================="
./ecdsa_test --check_sign=vmlinuz.img
echo "==========================================="






