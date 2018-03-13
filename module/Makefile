KDIR 		=/home/ruben/universidad/lse/kernel-rt/kernel/
CCC 		=/home/ruben/universidad/lse/gcc-linaro-arm-linux-gnueabihf-4.8-2013.06_linux/bin/arm-linux-gnueabihf-

obj-m += modEvent.o

default:
	make ARCH=arm CROSS_COMPILE=$(CCC) -C $(KDIR) M=`pwd` modules

clean:
	make ARCH=arm CROSS_COMPILE=$(CCC) -C $(KDIR) M=`pwd` clean
