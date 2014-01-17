#Matthew Yee, based on the given Makefile

ifneq ($(KERNELRELEASE),)
	obj-m := kRFID.o
else
	KERNELDIR := $(EC535)/gumstix/oe/linux-2.6.21/
	PWD := $(shell pwd)
	ARCH := arm
	CROSS := $(EC535)/gumstix/oe/cross/bin/arm-linux-

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) clean

endif
