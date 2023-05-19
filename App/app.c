#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>	// handles file management

#define DEVICE_NODE "/dev/vchar_dev"



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



int main() {
	int ret = 0;
	char option;
	int fd = -1;

	printf("Select one of below options:\n");
	printf("\to (to open a device node)\n");
	printf("\tc (to close the device node)\n");
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
