#include <linux/module.h>	// defines maroes as module_init and module_exit
#include <linux/fs.h>		// defines functions as allocate/release device number	
#include <linux/device.h>	// contains functions which handle device file
#include <linux/slab.h>		// contains functions as kmalloc and kzalloc
#include <linux/cdev.h>		// contains functions which work with cdev
#include <linux/uaccess.h>	// contains functions as copy_to_user, copy_from_user
#include <linux/ioctl.h>	// defines macroes as _IO, _IOWR,...
#include <linux/proc_fs.h>	// contains functions which create/remove file in procfs
#include <linux/timekeeping.h>	// contains functions to get wall time
#include <linux/jiffies.h>	// contains functions to get system uptime

#include "vchar_driver.h"	// defines registers of device

#define DRIVER_AUTHOR "danh21"
#define DRIVER_DESC "A sample character device driver"
#define DRIVER_VERSION "1.2"

#define MAGIC_NUM 21		// ID of driver
#define VCHAR_CLR_DATA_REGS 	_IO(MAGIC_NUM, 0)
#define VCHAR_GET_STT_REGS 	_IOR(MAGIC_NUM, 1, stt_regs_t *)
#define VCHAR_SET_RD_DATA_REGS 	_IOW(MAGIC_NUM, 2, unsigned char *)
#define VCHAR_SET_WR_DATA_REGS 	_IOW(MAGIC_NUM, 3, unsigned char *)

typedef struct {
	unsigned char read_count_h_reg;
	unsigned char read_count_l_reg;
	unsigned char write_count_h_reg;
	unsigned char write_count_l_reg;
	unsigned char device_status_reg;
} stt_regs_t; 

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
	unsigned long start_time;
} vchar_drv;



/****************************** Device Specific - START *****************************/
/* init device */
int vchar_hw_init(vchar_dev_t *hw) {
	char *mem;
	mem = kmalloc(NUM_DEV_REGS * REG_SIZE, GFP_KERNEL);
	if (!mem) {
		printk(KERN_ERR "Failed to allocate memory\n");
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
		printk(KERN_WARNING "Not allowed to read from data registers\n");
		return -1;
	}

	// check kernel buffer
	if (kbuf == NULL) {
		printk(KERN_ERR "Failed to allocate memory\n");
		return -ENOMEM;
	}

	// check start register
	if (start_reg > NUM_DATA_REGS) {
		printk(KERN_WARNING "Location of register to be read is invalid\n");
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
		printk(KERN_WARNING "Not allowed to write to data registers\n");
		return -1;
	}

	// check kernel buffer
	if (kbuf == NULL) {
		printk(KERN_ERR "Failed to allocate memory\n");
		return -ENOMEM;
	}

	// check start register
	if (start_reg > NUM_DATA_REGS) {
		printk(KERN_WARNING "Location of register to be read is invalid\n");
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

/* clear data regs */
int vchar_hw_clear_data(vchar_dev_t *hw) {
	if ((hw->ctrl_regs[CONTROL_ACCESS_REG] & CTRL_WRITE_DATA_BIT) == DISABLE) {
		printk(KERN_WARNING "Not allowed to clear data registers\n");
		return -1;
	}
	memset(hw->data_regs, 0, NUM_CTRL_REGS * REG_SIZE);
	hw->stt_regs[DEVICE_STATUS_REG] &= ~(STT_DATAREGS_OVERFLOW_BIT);
	return 0;
}

/* read status regs */
void vchar_hw_get_status(vchar_dev_t *hw, stt_regs_t *status) {
	memcpy(status, hw->stt_regs, NUM_STT_REGS * REG_SIZE);
}

/* enable or disable read access */
void vchar_hw_enable_read(vchar_dev_t *hw, unsigned char isEnable) {
	if (isEnable == ENABLE) {
		hw->ctrl_regs[CONTROL_ACCESS_REG] |= CTRL_READ_DATA_BIT;
		hw->stt_regs[DEVICE_STATUS_REG] |= STT_READ_ACCESS_BIT;	
	}
	else {
		hw->ctrl_regs[CONTROL_ACCESS_REG] &= ~CTRL_READ_DATA_BIT;
		hw->stt_regs[DEVICE_STATUS_REG] &= ~STT_READ_ACCESS_BIT;	
	}
}

/* enable or disable write access */
void vchar_hw_enable_write(vchar_dev_t *hw, unsigned char isEnable) {
	if (isEnable == ENABLE) {
		hw->ctrl_regs[CONTROL_ACCESS_REG] |= CTRL_WRITE_DATA_BIT;
		hw->stt_regs[DEVICE_STATUS_REG] |= STT_WRITE_ACCESS_BIT;	
	}
	else {
		hw->ctrl_regs[CONTROL_ACCESS_REG] &= ~CTRL_WRITE_DATA_BIT;
		hw->stt_regs[DEVICE_STATUS_REG] &= ~STT_WRITE_ACCESS_BIT;	
	}
}

/* ham xu ly tin hieu ngat gui tu thiet bi */

/******************************* device specific - END *****************************/



/******************************** OS specific - START *******************************/
/* entry points functions for device file */
static int vchar_driver_open(struct inode *inode, struct file *flip) {
	vchar_drv.start_time = jiffies;
	vchar_drv.open_cnt++;
	printk(KERN_INFO "Handle opened event (%d)\n", vchar_drv.open_cnt);
	return 0;
}

static int vchar_driver_release(struct inode *inode, struct file *flip) {
	struct timeval using_time;
	jiffies_to_timeval(jiffies - vchar_drv.start_time, &using_time);
	printk(KERN_INFO "The driver is used in %ld.%ld seconds\n", using_time.tv_sec, using_time.tv_usec/1000);
	printk(KERN_INFO "Handle closed event\n");
	return 0;
}

static ssize_t vchar_driver_read(struct file *flip, char __user *user_buf, size_t len, loff_t *off) {
	ssize_t num_bytes = 0;
	char *kernel_buf;
	struct timespec read_time = current_kernel_time();
	printk(KERN_INFO "Handle read %zu bytes event which starts from %lld at %ld.%ld from Epoch\n", len, *off, read_time.tv_sec, read_time.tv_nsec/1000000);	
		
	kernel_buf = kmalloc(len, GFP_KERNEL);
	if (kernel_buf == NULL) {
		printk(KERN_ERR "Failed to allocate memory\n");
		return -ENOMEM;
	}

	num_bytes = vchar_hw_read_data(vchar_drv.vchar_hw, *off, len, kernel_buf);
	printk(KERN_INFO "Read %zu bytes from device\n", num_bytes);
	if (num_bytes < 0) {
		printk(KERN_ERR "Failed to read data\n");
		return -EFAULT; // bad address
	}
	
	// Returns number of bytes that could not be copied. On success, this will be zero.
	if (copy_to_user(user_buf, kernel_buf, num_bytes)) {	
		printk(KERN_ERR "Failed to copy data to user\n");
		return -EFAULT;
	}
	
	*off += num_bytes;
	return num_bytes;
}

static ssize_t vchar_driver_write(struct file *flip, const char __user *user_buf, size_t len, loff_t *off) {
	ssize_t num_bytes = 0;
	char *kernel_buf;
	struct timeval write_time;
	do_gettimeofday(&write_time);
	printk(KERN_INFO "Handle write %zu bytes event which starts from %lld at %ld.%ld from Epoch\n", len, *off, write_time.tv_sec, write_time.tv_usec/1000);		
	
	kernel_buf = kmalloc(len, GFP_KERNEL);
	if (kernel_buf == NULL) {
		printk(KERN_ERR "Failed to allocate memory\n");
		return -ENOMEM;
	}

	// Returns number of bytes that could not be copied. On success, this will be zero.
	if (copy_from_user(kernel_buf, user_buf, len)) {	
		printk(KERN_ERR "Failed to copy data from user\n");
		return -EFAULT;
	}

	num_bytes = vchar_hw_write_data(vchar_drv.vchar_hw, *off, len, kernel_buf);
	printk(KERN_INFO "Wrote %zu bytes to device\n", num_bytes);
	if (num_bytes < 0) {
		printk(KERN_ERR "Failed to write data\n");
		return -EFAULT; // bad address
	}
	
	*off += num_bytes;
	return num_bytes;
}

static long vchar_driver_ioctl(struct file *flip, unsigned int cmd, unsigned long arg) {
	int ret = 0;
	stt_regs_t status;
	unsigned char isReadEnable, isWriteEnable;
	switch (cmd) {
		case VCHAR_CLR_DATA_REGS:
			ret = vchar_hw_clear_data(vchar_drv.vchar_hw);
			if (ret == 0)
				printk(KERN_INFO "Clear data registers successfully\n");
			else
				printk(KERN_WARNING "Can not clear data registers\n");
			break;
		case VCHAR_GET_STT_REGS:		
			vchar_hw_get_status(vchar_drv.vchar_hw, &status);
			if (copy_to_user((stt_regs_t *)arg, &status, sizeof(status))) {
				printk(KERN_ERR "Failed to send information of status registers to user\n");
				return -EFAULT;
			}
			printk(KERN_INFO "Got information from status registers\n");
			break;
		case VCHAR_SET_RD_DATA_REGS:		
			if (copy_from_user(&isReadEnable, (unsigned char*)arg, sizeof(isReadEnable))) {
				printk(KERN_ERR "Failed to get read access from user\n");
				return -EFAULT;
			}
			vchar_hw_enable_read(vchar_drv.vchar_hw, isReadEnable);
			printk(KERN_INFO "Set %s to read\n", (isReadEnable == 1) ? "enable" : "disable");
			break;
		case VCHAR_SET_WR_DATA_REGS:		
			if (copy_from_user(&isWriteEnable, (unsigned char*)arg, sizeof(isWriteEnable))) {
				printk(KERN_ERR "Failed to get write access from user\n");
				return -EFAULT;
			}
			vchar_hw_enable_write(vchar_drv.vchar_hw, isWriteEnable);
			printk(KERN_INFO "Set %s to write\n", (isWriteEnable == 1) ? "enable" : "disable");
			break;
	}
	return ret;
}

static struct file_operations fops = {
	.owner	 	= THIS_MODULE,
	.open 	 	= vchar_driver_open,
	.release 	= vchar_driver_release,
	.read 	 	= vchar_driver_read,
	.write 	 	= vchar_driver_write,
	.unlocked_ioctl = vchar_driver_ioctl
};



/* entry points functions for file in procfs */
static void* vchar_seq_start(struct seq_file *s, loff_t *pos) {
	char *msg;
	msg = kmalloc(256, GFP_KERNEL);
	if (!msg) {
		pr_err("seq_start: failed to allocate memory");
		return NULL;
	}
	sprintf(msg, "Message(%lld): size(%zu), from(%zu), count(%zu), index(%lld), read_pos(%lld)\n", *pos, s->size, s->from, s->count, s->index, s->read_pos);
	return msg;
}

static int vchar_seq_show(struct seq_file *s, void *pData) {
	char *msg = pData;
	seq_printf(s, "%s\n", msg);
	printk(KERN_INFO "seg_show: %s\n", msg);
	return 0;	// success
}

static void* vchar_seq_next(struct seq_file *s, void *pData, loff_t *pos) {
	char *msg = pData;
	++*pos;		// update next pos
	sprintf(msg, "Message(%lld): size(%zu), from(%zu), count(%zu), index(%lld), read_pos(%lld)\n", *pos, s->size, s->from, s->count, s->index, s->read_pos);
	return msg;
}

static void vchar_seq_stop(struct seq_file *s, void *pData) {
	printk(KERN_INFO "seq_stop\n");
	kfree(pData);
}

static struct seq_operations seq_ops = {
	.start = vchar_seq_start,
	.next  = vchar_seq_next,
	.stop  = vchar_seq_stop,
	.show  = vchar_seq_show
};


static int vchar_proc_open(struct inode *inode, struct file *flip) {
	printk(KERN_INFO "Handle opened event on proc file\n");
	return seq_open(flip, &seq_ops);
}

static int vchar_proc_release(struct inode *inode, struct file *flip) {
	printk(KERN_INFO "Handle closed event on proc file\n");
	return seq_release(inode, flip);
}

static ssize_t vchar_proc_read(struct file *flip, char __user *user_buf, size_t len, loff_t *off) {
	printk(KERN_INFO "Handle read %zu bytes event which starts from %lld on proc file\n", len, *off);		
	if (*off > 131072)	// user buffer of cat process has capacity: 131072 bytes
		printk(KERN_INFO "No problem about size of user buffer\n");
	return seq_read(flip, user_buf, len, off);
}

static ssize_t vchar_proc_write(struct file *flip, const char __user *user_buf, size_t len, loff_t *off) {
	printk(KERN_INFO "No action to handle writing event on proc file\n");	// read-only
	return len;
}

static struct file_operations proc_fops = {
	.open	 = vchar_proc_open,
	.release = vchar_proc_release,
	.read	 = vchar_proc_read,
	.write	 = vchar_proc_write	
};



/* init driver */
static int __init vchar_driver_init(void)
{
	int ret;	

	/* allocate device number */
	ret = alloc_chrdev_region(&vchar_drv.dev_num, 0, 1, "vchar_device");
	if (ret == 0)
		printk(KERN_INFO "Allocated device number (%d,%d) dynamically\n", MAJOR(vchar_drv.dev_num), MINOR(vchar_drv.dev_num));
	else {
		printk(KERN_ERR "Failed to register device number dynamically\n");
		goto failed_register_device_number;
	}	 

	/* create device class */
	vchar_drv.dev_class = class_create(THIS_MODULE, "class_vchar_dev");
	if (vchar_drv.dev_class == NULL) {
		printk(KERN_ERR "Failed to create a device class\n");
		ret = -1;
		goto failed_create_class;
	}

	/* create device file */
	vchar_drv.dev = device_create(vchar_drv.dev_class, NULL, vchar_drv.dev_num, NULL, "vchar_dev");
	if (IS_ERR(vchar_drv.dev)) {
		printk(KERN_ERR "Failed to create a device\n");
		ret = -1;
		goto failed_create_device;
	}

	/* allocate memory for struct of driver and init */
	vchar_drv.vchar_hw = kmalloc(sizeof(vchar_dev_t), GFP_KERNEL);
	if (!vchar_drv.vchar_hw) {
		printk(KERN_ERR "Failed to allocate memory for struct of driver\n");
		ret = -ENOMEM;		
		goto failed_alloc_struct;
	}

	/* init device */
	ret = vchar_hw_init(vchar_drv.vchar_hw);
	if (ret != 0) {
		printk(KERN_ERR "Failed to initialize hardware\n");
		goto failed_init_hw;
	}

	/* register entry points to kernel */
	vchar_drv.vcdev = cdev_alloc();
	if (vchar_drv.vcdev == NULL) {
		printk(KERN_ERR "Failed to allocate memory for cdev struct\n");
		ret = -ENOMEM;
		goto failed_alloc_cdev;
	}
	cdev_init(vchar_drv.vcdev, &fops);
	ret = cdev_add(vchar_drv.vcdev, vchar_drv.dev_num, 1);
	if (ret < 0) {
		printk(KERN_ERR "Failed to add char device to the system\n");
		goto failed_alloc_cdev;
	}

	/* Create file /proc/vchar_proc */
	if (proc_create("vchar_proc", 666, NULL, &proc_fops) == NULL) {
		printk(KERN_ERR "Failed to create file in procfs\n");
		goto failed_create_proc;
	}

	/* dang ky ham xu ly ngat */

	printk(KERN_INFO "Initialize virtual character driver successfully\n");
	return 0;

failed_create_proc:
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
	
	/* remove file /proc/vchar_proc */
	remove_proc_entry("vchar_proc", NULL);

	/* unregister entry point with kernel */
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

	printk(KERN_INFO "Exit virtual character driver\n");
}
/********************************* OS specific - END ********************************/



module_init(vchar_driver_init);
module_exit(vchar_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_SUPPORTED_DEVICE("testdevice");
