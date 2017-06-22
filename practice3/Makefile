obj-m:=taskmod.o
taskmod-y:=task.o

KERNELBUILD:=~/rpi2/linux/rpi-4.4.y
ccflags-y := -std=gnu99


module: task.c task.h
	$(MAKE) ARCH=arm -C $(KERNELBUILD)    M=$(shell pwd) modules ARCH=arm CROSS_COMPILE=armv7-rpi2-linux-gnueabihf-


clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
