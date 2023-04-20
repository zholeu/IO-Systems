EXTRA_CFLAGS+=-DLINUX_DRIVER -mhard-float
obj-m += calc.o 

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules  -I /usr/src/linux-headers-5.4.0-81-generic/include/
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean  -I /usr/src/linux-headers-5.4.0-81-generic/include/linux
