KDIR = /lib/modules/`uname -r`/build

all:
	make -C $(KDIR) M=`pwd` 	# compile modules in current directory

clean:
	make -C $(KDIR) M=$(PWD) clean 	# delete all object files in current directory
