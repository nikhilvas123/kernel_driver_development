#!/bin/bash
obj-m += pchar_driver.o
KSRC=/home/nikhil/Desktop/LTTS/training/kernel_driver/kernel/linux-5.14.7
# ARCH=arm
# CC=arm-linux-gnueabi-

all:
	make -C ${KSRC} M=${PWD} modules\
	ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-

clean:
	make -C ${KSRC} M=${PWD} clean\
	ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-