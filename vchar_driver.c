#include <linux/module.h>	// defines maroes as module_init and module_exit
#include <linux/fs.h>		// defines functions as allocate/release device number	
#include <linux/device.h>	// contains functions which handle device file
#include <linux/slab.h>		// contains functions as kmalloc and kzalloc

#include "vchar_driver.h"	// defines registers of device

#define DRIVER_AUTHOR "danh21"
#define DRIVER_DESC "A sample character device driver"
#define DRIVER_VERSION "0.5"



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
} vchar_drv;



/****************************** Device Specific - START *****************************/
/* init device */
int vchar_hw_init(vchar_dev_t *hw) {
	char *mem;
	mem = kmalloc(NUM_DEV_REGS * REG_SIZE, GFP_KERNEL);
	if (!mem) {
		printk("Failed to allocate memory\n");
		return -ENOMEM;
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

/* ham doc tu cac thanh ghi du lieu cua thiet bi */
/* ham ghi vao cac thanh ghi du lieu cua thiet bi */
/* ham doc tu cac thanh ghi trang thai cua thiet bi */
/* ham ghi vao cac thanh ghi dieu khien cua thiet bi */
/* ham xu ly tin hieu ngat gui tu thiet bi */
/******************************* device specific - END *****************************/



/******************************** OS specific - START *******************************/
/* cac ham entry points */

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

	/* dang ky cac entry point voi kernel */
	/* dang ky ham xu ly ngat */

	printk("Initialize virtual character driver successfully\n");
	return 0;

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
