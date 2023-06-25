#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>		// handles read write
#include <fcntl.h>		// handles open close
#include <string.h>
#include <sys/ioctl.h>		// handles ioctl

#define DEVICE_NODE "/dev/vchar_dev"
#define BUFFER_SIZE 1024

#define MAGIC_NUM 21		// ID of driver
#define CLR_DATA_CHARDEV	_IO(MAGIC_NUM, 0)
#define GET_STT_CHARDEV 	_IOR(MAGIC_NUM, 1, status_t *)
#define	SET_RD_ACCESS_CHARDEV	_IOW(MAGIC_NUM, 2, unsigned char *)
#define SET_WR_ACCESS_CHARDEV 	_IOW(MAGIC_NUM, 3, unsigned char *)

typedef struct {
	unsigned char read_count_h;
	unsigned char read_count_l;
	unsigned char write_count_h;
	unsigned char write_count_l;
	unsigned char device_status;
} status_t; 



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


/* check entry point ioctl of vchar driver */
void clear_data_chardev() {
	int fd = open_chardev();
	int ret = ioctl(fd, CLR_DATA_CHARDEV);
	close_chardev(fd);
	printf("Clear data in character device %s\n", (ret == 0) ? "successfully" : "unsuccessfully");
}

void get_stt_chardev() {
	status_t stt;
	int fd = open_chardev();
	int ret = ioctl(fd, GET_STT_CHARDEV, (status_t*) &stt);
	close_chardev(fd);

	if (ret == 0) {
		unsigned int read_cnt = (stt.read_count_h << 8) | stt.read_count_l;
		unsigned int write_cnt = (stt.write_count_h << 8) | stt.write_count_l;
		printf("Number of reading: %u\nNumber of writing: %u\n", read_cnt, write_cnt);
		printf("Device status: 0x%02x\n", stt.device_status);
	}
	else 
		printf("Get status from character device unsuccessfully\n");
}

void ctrl_read_chardev() {
	unsigned char isReadEnable, opt;
	printf("Do you want to enable to read from char device? (y/n): ");
	scanf(" %c", &opt);
	
	switch (opt) {
		case 'y':
			isReadEnable = 1;
			break;
		case 'n':
			isReadEnable = 0;
			break;
		default:
			printf("Invalid option\n");
			return;
	}
	
	int fd = open_chardev();
	int ret = ioctl(fd, SET_RD_ACCESS_CHARDEV, (unsigned char*) &isReadEnable);
	
	if (ret == 0) {	
		status_t stt;	
		ioctl(fd, GET_STT_CHARDEV, (status_t*) &stt);
		if (stt.device_status & 0x01)
			printf("Can read data from character device\n");
		else
			printf("Can not read data from character device\n");
	}
	else 
		printf("Modify read access of character device unsuccessfully\n");

	close_chardev(fd);
}

void ctrl_write_chardev() {
	unsigned char isWriteEnable, opt;
	printf("Do you want to enable to write to char device? (y/n): ");
	scanf(" %c", &opt);

	switch (opt) {
		case 'y':
			isWriteEnable = 1;
			break;
		case 'n':
			isWriteEnable = 0;
			break;
		default:
			printf("Invalid option\n");
			return;
	}
	
	int fd = open_chardev();
	int ret = ioctl(fd, SET_WR_ACCESS_CHARDEV, (unsigned char*) &isWriteEnable);
	
	if (ret == 0) {
		status_t stt;
		ioctl(fd, GET_STT_CHARDEV, (status_t*) &stt);
		if (stt.device_status & 0x02)
			printf("Can write data to character device\n");
		else
			printf("Can not write data to character device\n");
	}
	else 
		printf("Modify write access of character device unsuccessfully\n");

	close_chardev(fd);
}



int main() {
	int ret = 0;
	char option;
	int fd = -1;

	while (1) {
		printf(" ----------------- Select one of below options: ----------------\n");
		printf("\to (to open device node)\n");
		printf("\tc (to close device node)\n");
		printf("\tr (to read data from device node)\n");
		printf("\tw (to write data to device node)\n");
		printf("\tC (to clear data of device node)\n");
		printf("\ts (to get status of device node)\n");
		printf("\tR (to enable/disable to read from device node)\n");
		printf("\tW (to enable/disable to write to device node)\n");
		printf("\tq (to quit the application)\n");

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
			case 'C':
				clear_data_chardev();
				break;
			case 's':
				get_stt_chardev();
				break;
			case 'R':
				ctrl_read_chardev();
				break;
			case 'W':
				ctrl_write_chardev();
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
