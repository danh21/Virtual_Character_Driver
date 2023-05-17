KDIR = /lib/modules/`uname -r`/build

all:
	make -C $(KDIR) M=`pwd` # compile modules in current directory

# delete all object files in current directory
clean:
	#make -C $(KDIR) M=`pwd`clean 		# FAIL
	rm -f *.o *.symvers *.order *.ko *.mod.c
