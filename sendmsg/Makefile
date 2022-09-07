ifneq ($(KERNELRELEASE),)
obj-m :=netld.o
EXTRA_CFLAGS += -g
else
KERNELDIR ?=/lib/modules/$(shell uname -r)/build
PWD :=$(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
