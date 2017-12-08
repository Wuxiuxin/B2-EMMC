#!/bin/bash

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

make distclean
make am335x_evm_config
make -j4

