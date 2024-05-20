#!/bin/bash

SIZE=512

cd ./src && make
insmod dmp.ko

echo "Creating devices"
dmsetup create zero1 --table "0 ${SIZE} zero"
dmsetup create dmp1 --table "0 ${SIZE} dmp /dev/mapper/zero1"

echo "Testing devices"
dd if=/dev/random of=/dev/mapper/dmp1 bs=4k count=1
dd of=/dev/null if=/dev/mapper/dmp1 bs=4k count=1

echo "Cleaning"
dmsetup remove dmp1
dmsetup remove zero1
rmmod dmp
make clean
