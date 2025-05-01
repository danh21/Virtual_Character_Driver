#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_NODE "/dev/vchar_dev"
#define MAGIC_NUM 21
#define CHANGE_DATA_IN_CRITICAL_RESOURCE _IO(MAGIC_NUM, 4)

int main()
{
	int fd;
	unsigned int loop = 1 << 20;	//1,048,576

	fd = open(DEVICE_NODE, O_RDWR);
	if(fd < 0) {
		printf("Cannot open device file...\n");
		return -1;
	}

	while(loop--)
		ioctl(fd, CHANGE_DATA_IN_CRITICAL_RESOURCE);

	close(fd);
	return 0;
}
