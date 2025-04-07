MODULE_NAME := kfetch_mod_313551099
PWD := $(shell pwd)

obj-m += $(MODULE_NAME).o
EXTRA_CFLAGS := -w

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

load:
	sudo insmod $(MODULE_NAME).ko

unload:
	sudo rmmod $(MODULE_NAME)

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
