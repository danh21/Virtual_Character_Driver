#include <linux/module.h>	// defines maroes as module_init and module_exit
#include <linux/fs.h>		// defines functions as allocate/release device number	
#include <linux/device.h>	// contains functions which handle device file
#include <linux/slab.h>		// contains functions as kmalloc and kzalloc
#include <linux/cdev.h>		// contains functions which work with cdev
#include <linux/uaccess.h>	// contains functions as copy_to_user, copy_from_user

#include "vchar_driver.h"	// defines registers of device

#define DRIVER_AUTHOR "danh21"
#define DRIVER_DESC "A sample character device driver"
#define DRIVER_VERSION "0.7"



typedef struct {
	unsigned char *ctrl_regs;
	unsigned char *stt_regs;
	unsigned char *data_regs;
} vchar_dev_t;

struct _vchar_drv {
	dev_t dev_num;
	struct class *dev_class;
	struct device *dev;
	vchar_dev_t *vchar_hw;
	struct cdev *vcdev;
	unsigned int open_cnt;
} vchar_drv;



/****************************** Device Specific - START *****************************/
/* init device */
int vchar_hw_init(vchar_dev_t *hw) {
	char *mem;
	mem = kmalloc(NUM_DEV_REGS * REG_SIZE, GFP_KERNEL);
	if (!mem) {
		printk("Failed to allocate memory\n");
		return -ENOMEM; // out of memory
	}

	hw->ctrl_regs = mem;
	hw->stt_regs = hw->ctrl_regs + NUM_CTRL_REGS;
	hw->data_regs = hw->stt_regs + NUM_STT_REGS;

	hw->ctrl_regs[CONTROL_ACCESS_REG] = 0x03;
	hw->stt_regs[DEVICE_STATUS_REG] = 0x03;

	return 0;
}


/* release device */
void vchar_hw_exit(vchar_dev_t *hw) {
	kfree(hw->ctrl_regs);
}


/* read from data regs of device */
int vchar_hw_read_data(vchar_dev_t *hw, int start_reg, int num_regs, char *kbuf) {
	int read_bytes = num_regs;

	// check control register
	if ((hw->ctrl_regs[CONTROL_ACCESS_REG] & CTRL_READ_DATA_BIT) == 0) {
		printk("Not allowed to read from data registers\n");
		return -1;
	}

	// check kernel buffer
	if (kbuf == NULL) {
		printk("Failed to allocate memory\n");
		return -ENOMEM;
	}

	// check start register
	if (start_reg > NUM_DATA_REGS) {
		printk("Location of register to be read is invalid\n");
		return -1;
	}

	// check number of bytes to be read
	if (read_bytes > (NUM_DATA_REGS - start_reg)) 
		read_bytes = NUM_DATA_REGS - start_reg;
	
	// write data from data register to kernel buffer
	memcpy(kbuf, hw->data_regs + start_reg, read_bytes);

	// update status register
	hw->stt_regs[READ_COUNT_L_REG]++;
	if (hw->stt_regs[READ_COUNT_L_REG] == 0)
		hw->stt_regs[READ_COUNT_H_REG]++;

	return read_bytes;
}


/* write to data regs of device */
int vchar_hw_write_data(vchar_dev_t *hw, int start_reg, int num_regs, char *kbuf) {
	int write_bytes = num_regs;

	// check control register
	if ((hw->ctrl_regs[CONTROL_ACCESS_REG] & CTRL_WRITE_DATA_BIT) == 0) {
		printk("Not allowed to write to data registers\n");
		return -1;
	}

	// check kernel buffer
	if (kbuf == NULL) {
		printk("Failed to allocate memory\n");
		return -ENOMEM;
	}

	// check start register
	if (start_reg > NUM_DATA_REGS) {
		printk("Location of register to be read is invalid\n");
		return -1;
	}

	// check number of bytes to be written
	if (write_bytes > (NUM_DATA_REGS - start_reg)) {
		write_bytes = NUM_DATA_REGS - start_reg;
		hw->stt_regs[DEVICE_STATUS_REG] |= STT_DATAREGS_OVERFLOW_BIT;
	}
		
	
	// write data to data register from kernel buffer
	memcpy(hw->data_regs + start_reg, kbuf, write_bytes);

	// update status register
	hw->stt_regs[WRITE_COUNT_L_REG]++;
	if (hw->stt_regs[WRITE_COUNT_L_REG] == 0)
		hw->stt_regs[WRITE_COUNT_H_REG]++;

	return write_bytes;
}


/* ham doc tu cac thanh ghi trang thai cua thiet bi */
/* ham ghi vao cac thanh ghi dieu khien cua thiet bi */
/* ham xu ly tin hieu ngat gui tu thiet bi */
/******************************* device specific - END *****************************/



/******************************** OS specific - START *******************************/
/* entry points functions*/
static int vchar_driver_open(struct inode *inode, struct file *flip) {
	vchar_drv.open_cnt++;
	printk("Handle opened event (%d)\n", vchar_drv.open_cnt);
	return 0;
}

static int vchar_driver_release(struct inode *inode, struct file *flip) {
	printk("Handle closed event\n");
	return 0;
}

static ssize_t vchar_driver_read(struct file *flip, char __user *user_buf, size_t len, loff_t *off) {
	ssize_t num_bytes = 0;
	printk("Handle read %zu bytes event which starts from %lld\n", len, *off);	
	
	char *kernel_buf;
	kernel_buf = kmalloc(len, GFP_KERNEL);
	if (kernel_buf == NULL) {
		printk("Failed to allocate memory\n");
		return -ENOMEM;
	}

	num_bytes = vchar_hw_read_data(vchar_drv.vchar_hw, *off, len, kernel_buf);
	printk("Read %zu bytes from device\n", num_bytes);
	if (num_bytes < 0) {
		printk("Failed to read data\n");
		return -EFAULT; // bad address
	}
	
	// Returns number of bytes that could not be copied. On success, this will be zero.
	if (copy_to_user(user_buf, kernel_buf, num_bytes)) {	
		printk("Failed to copy data to user\n");
		return -EFAULT;
	}
	
	*off += num_bytes;
	return num_bytes;
}

static ssize_t vchar_driver_write(struct file *flip, const char __user *user_buf, size_t len, loff_t *off) {
	ssize_t num_bytes = 0;
	printk("Handle write %zu bytes event which starts from %lld\n", len, *off);	
	
	char *kernel_buf;
	kernel_buf = kmalloc(len, GFP_KERNEL);
	if (kernel_buf == NULL) {
		printk("Failed to allocate memory\n");
		return -ENOMEM;
	}

	// Returns number of bytes that could not be copied. On success, this will be zero.
	if (copy_from_user(kernel_buf, user_buf, len)) {	
		printk("Failed to copy data from user\n");
		return -EFAULT;
	}

	num_bytes = vchar_hw_write_data(vchar_drv.vchar_hw, *off, len, kernel_buf);
	printk("Write %zu bytes to device\n", num_bytes);
	if (num_bytes < 0) {
		printk("Failed to write data\n");
		return -EFAULT; // bad address
	}
	
	*off += num_bytes;
	return num_bytes;
}


static struct file_operations fops = {
	.owner	 = THIS_MODULE,
	.open 	 = vchar_driver_open,
	.release = vchar_driver_release,
	.read 	 = vchar_driver_read,
	.write 	 = vchar_driver_write
};


/* init driver */
static int __init vchar_driver_init(void)
{
	int ret;	

	/* allocate device number */
	ret = alloc_chrdev_region(&vchar_drv.dev_num, 0, 1, "vchar_device");
	if (ret == 0)
		printk("Allocated device number (%d,%d) dynamically\n", MAJOR(vchar_drv.dev_num), MINOR(vchar_drv.dev_num));
	else {
		printk("Failed to register device number dynamically\n");
		goto failed_register_device_number;
	}	 

	/* create device class */
	vchar_drv.dev_class = class_create(THIS_MODULE, "class_vchar_dev");
	if (vchar_drv.dev_class == NULL) {
		printk("Failed to create a device class\n");
		ret = -1;
		goto failed_create_class;
	}

	/* create device file */
	vchar_drv.dev = device_create(vchar_drv.dev_class, NULL, vchar_drv.dev_num, NULL, "vchar_dev");
	if (IS_ERR(vchar_drv.dev)) {
		printk("Failed to create a device\n");
		ret = -1;
		goto failed_create_device;
	}

	/* allocate memory for struct of driver and init */
	vchar_drv.vchar_hw = kmalloc(sizeof(vchar_dev_t), GFP_KERNEL);
	if (!vchar_drv.vchar_hw) {
		printk("Failed to allocate memory for struct of driver\n");
		ret = -ENOMEM;		
		goto failed_alloc_struct;
	}

	/* init device */
	ret = vchar_hw_init(vchar_drv.vchar_hw);
	if (ret != 0) {
		printk("Failed to initialize hardware\n");
		goto failed_init_hw;
	}

	/* register entry points to kernel */
	vchar_drv.vcdev = cdev_alloc();
	if (vchar_drv.vcdev == NULL) {
		printk("Failed to allocate memory for cdev struct\n");
		ret = -ENOMEM;
		goto failed_alloc_cdev;
	}
	cdev_init(vchar_drv.vcdev, &fops);
	ret = cdev_add(vchar_drv.vcdev, vchar_drv.dev_num, 1);
	if (ret < 0) {
		printk("Failed to add char device to the system\n");
		goto failed_alloc_cdev;
	}

	/* dang ky ham xu ly ngat */


	printk("Initialize virtual character driver successfully\n");
	return 0;

failed_alloc_cdev:
	vchar_hw_exit(vchar_drv.vchar_hw);
failed_init_hw:
	kfree(vchar_drv.vchar_hw);
failed_alloc_struct:
	device_destroy(vchar_drv.dev_class, vchar_drv.dev_num);
failed_create_device:
	class_destroy(vchar_drv.dev_class);
failed_create_class:
	unregister_chrdev_region(vchar_drv.dev_num, 1);
failed_register_device_number:
	return ret;	
}


/* exit driver */
static void __exit vchar_driver_exit(void)
{
	/* huy dang ky xu ly ngat */
	/* huy dang ky entry point voi kernel */
	cdev_del(vchar_drv.vcdev);

	/* release device */
	vchar_hw_exit(vchar_drv.vchar_hw);

	/* release memory of struct of the driver */
	kfree(vchar_drv.vchar_hw);

	/* delete device file */
	device_destroy(vchar_drv.dev_class, vchar_drv.dev_num);
	class_destroy(vchar_drv.dev_class);

	/* release device number */
	unregister_chrdev_region(vchar_drv.dev_num, 1);

	printk("Exit virtual character driver\n");
}
/********************************* OS specific - END ********************************/



module_init(vchar_driver_init);
module_exit(vchar_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_SUPPORTED_DEVICE("testdevice");
