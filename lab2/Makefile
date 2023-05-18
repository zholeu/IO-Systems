EXTRA_CFLAGS+=-DLINUX_DRIVER -mhard-float
obj-m += io2.o

all:
    make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules  -I /usr/src/linux-headers-5.15.0-41-generic/include/
clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean  -I /usr/src/linux-headers-5.15.0-41-generic/include/linux
