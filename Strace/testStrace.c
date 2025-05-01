#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_NODE "/dev/vchar_dev"
int8_t read_buf[1024];
int8_t write_buf[1024] = "hello DANH";

int main()
{
	int fd = open(DEVICE_NODE, O_RDWR);
	if (fd < 0) {
		printf("Cannot open device file...\n");
		return 0;
	}

	/* writes bytes from the buffer to the file descriptor at offset */
	pwrite(fd, write_buf, strlen(write_buf)+1, 0);

	/* reads bytes from file descriptor at offset (from the start of the file) into the buffer */
	pread(fd, read_buf, sizeof(read_buf), 0);

	/* Display data */
	printf("Data = %s\n", read_buf);

	close(fd);
	return 0;
}
