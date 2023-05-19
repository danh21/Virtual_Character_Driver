#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	// handles read write
#include <fcntl.h>	// handles open close
#include <string.h>

#define DEVICE_NODE "/dev/vchar_dev"
#define BUFFER_SIZE 1024



/* check entry point open of vchar driver */
int open_chardev() {
	int fd = open(DEVICE_NODE, O_RDWR); 
	// fd: file descriptor same as id
	// O_RDWR: Opens or creates a file with read and write access.
	if(fd < 0) {
		printf("Can not open the device file\n");
		exit(1);
	}
	return fd;
}

/* check entry point release of vchar driver */
void close_chardev(int fd) {
	close(fd);
}

/* check entry point read of vchar driver */
void read_chardev() {
	int ret = 0;
	char user_buf[BUFFER_SIZE];

	int fd = open_chardev();
	ret = read(fd, user_buf, BUFFER_SIZE);
	close_chardev(fd);

	if (ret < 0)
		printf("Can not read data from %s\n", DEVICE_NODE);
	else
		printf("Read data from %s successfully: %s\n", DEVICE_NODE, user_buf);
}

/* check entry point write of vchar driver */
void write_chardev() {
	int ret = 0;
	char user_buf[BUFFER_SIZE];

	printf("Enter data: ");
	scanf(" %[^\n]s", user_buf);

	int fd = open_chardev();
	ret = write(fd, user_buf, strlen(user_buf) + 1); // + Null
	close_chardev(fd);

	if (ret < 0)
		printf("Can not write data to %s\n", DEVICE_NODE);
	else
		printf("Wrote data to %s successfully\n", DEVICE_NODE);
}



int main() {
	int ret = 0;
	char option;
	int fd = -1;

	printf("Select one of below options:\n");
	printf("\to (to open device node)\n");
	printf("\tc (to close device node)\n");
	printf("\tr (to read data from device node)\n");
	printf("\tw (to write data to device node)\n");
	printf("\tq (to quit the application)\n");

	while (1) {
		printf("Enter your option: ");
		scanf(" %c", &option);	// NOTICE
		switch (option) {
			case 'o':
				if (fd < 0)
					fd = open_chardev();
				else
					printf("%s has already opened\n", DEVICE_NODE);
				break;
			case 'c':
				if (fd > -1)
					close_chardev(fd);
				else
					printf("%s has not opened yet!\n", DEVICE_NODE);
				fd = -1;
				break;
			case 'r':
				read_chardev();
				break;
			case 'w':
				write_chardev();
				break;	
			case 'q':
				if (fd > -1)
					close_chardev(fd);
				printf("Quit the application. Good bye!\n");
				return 0;		
			default:
				printf("Invalid option\n");
				break;
		}
	};
}
