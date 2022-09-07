ifneq ($(KERNELRELEASE),)
obj-m :=netlink_d.o
EXTRA_CFLAGS += -g
else
KERNELDIR ?=/lib/modules/$(shell uname -r)/build
PWD :=$(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
