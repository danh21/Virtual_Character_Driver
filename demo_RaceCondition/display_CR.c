#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_NODE "/dev/vchar_dev"
#define MAGICAL_NUM 21
#define DISPLAY_DATA_IN_CRITICAL_RESOURCE _IO(MAGICAL_NUM, 5)

int main()
{
	int fd;

	fd = open(DEVICE_NODE, O_RDWR);
	if(fd < 0) {
		printf("Cannot open device file...\n");
		return -1;
	}

	ioctl(fd, DISPLAY_DATA_IN_CRITICAL_RESOURCE);

	close(fd);
	return 0;
}
