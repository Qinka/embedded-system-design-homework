obj-m := src/led.o

KERNELBUILD:=~/rpi2/linux-rpi-4.4.y

all:
	$(MAKE) ARCH=arm -C $(KERNELBUILD) M=$(shell pwd) modules ARCH=arm CROSS_COMPILE=armv7-rpi2-linux-gnueabihf-  INCLUDE_PATH+=' include/'

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
