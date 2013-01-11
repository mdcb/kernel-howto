PWD := $(shell pwd)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

all: nlclient modules

nlclient: nlclient.c
	$(CC) -Wall `pkg-config --cflags --libs libnl-genl-3.0` -o $@ $<

modules: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) clean
	rm -f nlclient

install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install


