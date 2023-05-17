KDIR = /lib/modules/`uname -r`/build

all:
	make -C $(KDIR) M=`pwd` # biên dịch các module trong thư mục hiện tại

# xóa tất cả các object file có trong thư mục hiện tại
clean:
	#make -C $(KDIR) M=`pwd`clean 		# FAIL
	rm -f *.o *.symvers *.order *.ko *.mod.c
