#include <linux/module.h>	// defines maroes as module_init and module_exit
#include <linux/fs.h>		// defines functions as allocate/release device number	
#include <linux/device.h>	// contains functions which handle device file

#define DRIVER_AUTHOR "danh21"
#define DRIVER_DESC "A sample character device driver"
#define DRIVER_VERSION "0.4"

struct _vchar_drv {
	dev_t dev_num;
	struct class *dev_class;
	struct device *dev;
} vchar_drv;



/****************************** Device Specific - START *****************************/
/* ham khoi tao thiet bi */
/* ham giai phong thiet bi */
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
	/* allocate device number */
	if (alloc_chrdev_region(&vchar_drv.dev_num, 0, 1, "vchar_device") == 0)
		printk("Allocated device number (%d,%d) dynamically\n", MAJOR(vchar_drv.dev_num), MINOR(vchar_drv.dev_num));
	else {
		printk("Failed to register device number dynamically\n");
		goto failed_register_device_number;
	}	 

	/* create device class */
	vchar_drv.dev_class = class_create(THIS_MODULE, "class_vchar_dev");
	if (vchar_drv.dev_class == NULL) {
		printk("Failed to create a device class\n");
		goto failed_create_class;
	}

	/* create device file */
	vchar_drv.dev = device_create(vchar_drv.dev_class, NULL, vchar_drv.dev_num, NULL, "vchar_dev");
	if (IS_ERR(vchar_drv.dev)) {
		printk("Failed to create a device\n");
		goto failed_create_device;
	}

	/* cap phat bo nho cho cac cau truc du lieu cua driver va khoi tao */
	/* khoi tao thiet bi vat ly */
	/* dang ky cac entry point voi kernel */
	/* dang ky ham xu ly ngat */

	printk("Initialize virtual character driver successfully\n");
	return 0;

failed_create_device:
	class_destroy(vchar_drv.dev_class);
failed_create_class:
	unregister_chrdev_region(vchar_drv.dev_num, 1);
failed_register_device_number:
	return -1;	
}

/* exit driver */
static void __exit vchar_driver_exit(void)
{
	/* huy dang ky xu ly ngat */
	/* huy dang ky entry point voi kernel */
	/* giai phong thiet bi vat ly */
	/* giai phong bo nho da cap phat cau truc du lieu cua driver */

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
