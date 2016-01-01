#Reference: http://www.devdrv.co.jp/linux/kernel26-makefile.htm
TARGET:= kiken.ko

all: ${TARGET}

kiken.ko: kiken.c
	make -C /usr/src/linux-headers-`uname -r` M=`pwd` V=1 modules

clean:
	make -C /usr/src/linux-headers-`uname -r` M=`pwd` V=1 clean

obj-m:= kiken.o

clean-files := *.o *.ko *.mod.[co] *~
