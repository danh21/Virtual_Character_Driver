#include <linux/module.h>	// defines maroes as module_init and module_exit
#include <linux/fs.h>		// defines functions as allocate/release device number	
#include <linux/device.h>	// contains functions which handle device file
#include <linux/slab.h>		// contains functions which are related to kernel memory allocation
#include <linux/cdev.h>		// contains functions which work with cdev
#include <linux/uaccess.h>	// contains functions as copy_to_user, copy_from_user
#include <linux/ioctl.h>	// defines macroes as _IO, _IOWR,...
#include <linux/proc_fs.h>	// contains functions which create/remove file in procfs
#include <linux/timekeeping.h>	// contains functions to get wall time
#include <linux/jiffies.h>	// contains functions to get system uptime
#include <linux/sched.h>	// contains functions which are related to scheduling
#include <linux/delay.h>	// contains functions which are related to delay and sleep
//#include <linux/timer.h>	// contains functions which are related to kernel timer
//#include <linux/interrupt.h>	// contains functions which are related to interrupt
//#include <linux/workqueue.h>	// contains functions which are related to workqueue
//#include <linux/spinlock.h>	// contains functions which are related to spinlock
#include <linux/mutex.h>	// contains functions which are related to mutex
//#include <linux/semaphore.h>	// contains functions which are related to semaphore
//#include <linux/vmalloc.h>	// contains functions as vmalloc and vfree
//#include <linux/gfp.h>	// contains functions as get_zeroed_page
#include <linux/ioport.h>	// contains functions which are related to Port IO
#include <linux/mm.h>		// contains functions which are related to memory mapping

#include "vchar_driver.h"	// defines registers of device



#define DRIVER_AUTHOR "danh21"
#define DRIVER_DESC "A sample character device driver"
#define DRIVER_VERSION "2.8"

#define MAGIC_NUM 21		// ID of driver
#define VCHAR_CLR_DATA_REGS 			_IO (MAGIC_NUM, 0)
#define VCHAR_GET_STT_REGS 			_IOR(MAGIC_NUM, 1, stt_regs_t *)
#define VCHAR_SET_RD_DATA_REGS 			_IOW(MAGIC_NUM, 2, unsigned char *)
#define VCHAR_SET_WR_DATA_REGS 			_IOW(MAGIC_NUM, 3, unsigned char *)
#define VCHAR_CHANGE_DATA_IN_CRITICAL_RESOURCE	_IO (MAGIC_NUM, 4)
#define VCHAR_DISPLAY_DATA_IN_CRITICAL_RESOURCE	_IO (MAGIC_NUM, 5)
#define VCHAR_RESET_DATA_IN_CRITICAL_RESOURCE	_IO (MAGIC_NUM, 6)

#define IRQ_NUMBER 11

#define OBJ_LOOKASIDE_SIZE (NUM_DATA_REGS * REG_SIZE)

#define VCHAR_IOPORT_BASE	0x30
#define VCHAR_IOPORT_LENGTH 	16	// 0x30 -> 0x3F
#define VCHAR_IOPORT_NAME	"vchar_port"
#define VCHAR_IOPORT(offset)	(VCHAR_IOPORT_BASE + offset)



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
	dev_t dev_num;						// device number
	struct class *dev_class;				// class of device
	struct device *dev;					// device
	vchar_dev_t *vchar_hw;					// contains registers of device
	struct cdev *vcdev;					// for device file operations
	unsigned int open_cnt;					// number of times to open device file
	unsigned long start_time;				// time which starts opening device file
	struct timer_list vchar_ktimer;				// kernel timer
	volatile uint32_t intr_cnt;				// interrupt counter
	struct tasklet_struct* vchar_dynamic_tasklet;		// dynamic tasklet
	struct workqueue_struct* vchar_user_workqueue;		// user-defined workqueue	
	unsigned int critical_resource;				// data in critical resource
	//atomic_t critical_resource;				// data in critical resource
	//spinlock_t vchar_spinlock;				// spinlock protects critical resource
	struct mutex vchar_mutexlock;				// mutex lock ptotects critical resource
	//struct semaphore vchar_semaphore;			// semaphore ptotects critical resource
	struct kmem_cache *vchar_lookaside_cache;		// lookaside cache 
	struct resource *vchar_ioport;				// manage port region
} vchar_drv; 

typedef struct {	// for task of kernel timer
	int param1;
	int param2;
} vchar_ktimer_data_t;



/* *************************************************************************************** Device Specific - START ************************************************************************************** */
/* init device */
int vchar_hw_init(vchar_dev_t *hw) {
	char *mem;
	// mem = kmalloc(NUM_DEV_REGS * REG_SIZE, GFP_KERNEL);
	// mem = vmalloc(NUM_DEV_REGS * REG_SIZE);
	// mem = (char *)get_zeroed_page(GFP_KERNEL);
	mem = kmalloc((NUM_CTRL_REGS + NUM_STT_REGS) * REG_SIZE, GFP_KERNEL);
	if (!mem) {
		printk(KERN_ERR "Failed to allocate memory\n");
		return -ENOMEM; // out of memory
	}

	hw->ctrl_regs = mem;
	hw->stt_regs = hw->ctrl_regs + NUM_CTRL_REGS;
	// hw->data_regs = hw->stt_regs + NUM_STT_REGS;
	hw->data_regs = (char *)get_zeroed_page(GFP_KERNEL);

	hw->ctrl_regs[CONTROL_ACCESS_REG] = 0x03;
	hw->stt_regs[DEVICE_STATUS_REG] = 0x03;
	return 0;
}



/* release device */
void vchar_hw_exit(vchar_dev_t *hw) {
	kfree(hw->ctrl_regs);
	// vfree(hw->ctrl_regs);
	// free_page((unsigned long)hw->ctrl_regs);
	free_page((unsigned long)hw->data_regs);
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



/* ISR from device */

/* 
void vchar_hw_bh_task(unsigned long arg) {
	uint32_t* intr_cnt_p = (uint32_t*)arg;
	printk(KERN_INFO "[Bottom-half task] [CPU %d] interrupt counter: %d\n", smp_processor_id(), *intr_cnt_p);
}
DECLARE_TASKLET(vchar_static_tasklet, vchar_hw_bh_task, (unsigned long)&vchar_drv.intr_cnt); 
*/

/*
void vchar_hw_bh_task(struct work_struct* task) {
	printk(KERN_INFO "[Bottom-half task] [CPU %d] ... \n", smp_processor_id());
}
DECLARE_WORK(vchar_static_work, vchar_hw_bh_task);
*/

/*
irqreturn_t vchar_hw_isr(int irq, void* dev) {
	// ----------------- top-half task
	vchar_drv.intr_cnt++;

	// ----------------- bottom-half task

	// tasklet_schedule(&vchar_static_tasklet);
	// tasklet_schedule(vchar_drv.vchar_dynamic_tasklet);	
	
	// -------- default workqueue
	// schedule_work_on(0, &vchar_static_work);
	// schedule_delayed_work_on(1, &vchar_static_delayed_work, 2*HZ);

	// -------- user-defined workqueue
	//queue_work_on(0, vchar_drv.vchar_user_workqueue, &vchar_static_work);
	//queue_delayed_work_on(1, vchar_drv.vchar_user_workqueue, &vchar_static_delayed_work, 5*HZ);

	return IRQ_HANDLED;
}
*/
/* **************************************************************************************** Device specific - END ************************************************************************************** */



/* ***************************************************************************************** OS specific - START **************************************************************************************** */
/* ***************************** entry points functions for device file ***************************** */
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
		
	// kernel_buf = kmalloc(len, GFP_KERNEL);
	// kernel_buf = vmalloc(len);
	kernel_buf = kmem_cache_alloc(vchar_drv.vchar_lookaside_cache, GFP_KERNEL);
	if (kernel_buf == NULL) {
		printk(KERN_ERR "Failed to allocate memory\n");
		return -ENOMEM;
	}

	// driver will sleep for a period of at least timeout
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(1 * HZ); // 1s

	// READ
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

	kmem_cache_free(vchar_drv.vchar_lookaside_cache, kernel_buf);
	*off += num_bytes;
	return num_bytes;
}



static ssize_t vchar_driver_write(struct file *flip, const char __user *user_buf, size_t len, loff_t *off) {
	ssize_t num_bytes = 0;
	char *kernel_buf;

	struct timeval write_time;
	do_gettimeofday(&write_time);
	printk(KERN_INFO "Handle write %zu bytes event which starts from %lld at %ld.%ld from Epoch\n", len, *off, write_time.tv_sec, write_time.tv_usec/1000);		
	
	// kernel_buf = kmalloc(len, GFP_KERNEL);
	// kernel_buf = vmalloc(len);
	kernel_buf = kmem_cache_alloc(vchar_drv.vchar_lookaside_cache, GFP_KERNEL);
	if (kernel_buf == NULL) {
		printk(KERN_ERR "Failed to allocate memory\n");
		return -ENOMEM;
	}

	// Returns number of bytes that could not be copied. On success, this will be zero.
	if (copy_from_user(kernel_buf, user_buf, len)) {	
		printk(KERN_ERR "Failed to copy data from user\n");
		return -EFAULT;
	}

	//mdelay(1000);	//1s
	ssleep(1);	//1s
	
	// WRITE
	num_bytes = vchar_hw_write_data(vchar_drv.vchar_hw, *off, len, kernel_buf);
	printk(KERN_INFO "Wrote %zu bytes to device\n", num_bytes);
	if (num_bytes < 0) {
		printk(KERN_ERR "Failed to write data\n");
		return -EFAULT; // bad address
	}
	
	kmem_cache_free(vchar_drv.vchar_lookaside_cache, kernel_buf);
	*off += num_bytes;
	return num_bytes;
}



static int vchar_driver_mmap(struct file *file, struct vm_area_struct *vma) {
	unsigned long offset, mapped_area_size, phy_addr, pfn, data_mem_size = PAGE_SIZE;
	
	// determine location on mapped physical memory
	offset = vma->vm_pgoff << PAGE_SHIFT;
	if (offset > data_mem_size) {
		printk(KERN_ERR "Failed offset on mapped physical memory\n");
		return -EINVAL;
	}
	
	// size of mapped_area_size
	mapped_area_size = vma->vm_end - vma->vm_start;
	if (mapped_area_size > data_mem_size - offset) {
		printk(KERN_ERR "Failed size of mapped physical memory\n");
		return -EINVAL;
	}

	// determine PFN of page frame which contains offset
	phy_addr = virt_to_phys(vchar_drv.vchar_hw->data_regs + offset);
	pfn = phy_addr >> PAGE_SHIFT;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);	// not used cache

	if (io_remap_pfn_range(vma, vma->vm_start, pfn, mapped_area_size, vma->vm_page_prot)) {
		printk(KERN_ERR "Failed mapping\n");		
		return -EAGAIN;
	}
	
	return 0;	
}



static long vchar_driver_ioctl(struct file *flip, unsigned int cmd, unsigned long arg) {
	int ret = 0;
	stt_regs_t status;
	unsigned char isReadEnable, isWriteEnable;
	printk(KERN_INFO "Handle ioctl event with command (%u)\n", cmd);
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
		case VCHAR_CHANGE_DATA_IN_CRITICAL_RESOURCE:
			// --------------------------- atomic method			
			// atomic_inc(&vchar_drv.critical_resource);
			
			// --------------------------- spinlock method
			/*
			spin_lock(&vchar_drv.vchar_spinlock);
			vchar_drv.critical_resource++;
			spin_unlock(&vchar_drv.vchar_spinlock);
			*/
	
			// --------------------------- mutex method
						
			mutex_lock(&vchar_drv.vchar_mutexlock);
			vchar_drv.critical_resource++;
			mutex_unlock(&vchar_drv.vchar_mutexlock);
			

			// --------------------------- semaphore method			
			/*			
			down(&vchar_drv.vchar_semaphore);
			vchar_drv.critical_resource++;
			up(&vchar_drv.vchar_semaphore);
			*/
			break;
		case VCHAR_DISPLAY_DATA_IN_CRITICAL_RESOURCE:			
			printk(KERN_INFO "Data in critical resource: %d\n", vchar_drv.critical_resource);
			// printk(KERN_INFO "Data in critical resource: %d\n", atomic_read(&vchar_drv.critical_resource));
			break;
		case VCHAR_RESET_DATA_IN_CRITICAL_RESOURCE:
			// --------------------------- atomic method
			// atomic_set(&vchar_drv.critical_resource, 0);

			// --------------------------- spinlock method
			/*			
			spin_lock(&vchar_drv.vchar_spinlock);
			vchar_drv.critical_resource = 0;
			spin_unlock(&vchar_drv.vchar_spinlock);
			*/

			// --------------------------- mutex method
			
			mutex_lock(&vchar_drv.vchar_mutexlock);
			vchar_drv.critical_resource = 0;
			mutex_unlock(&vchar_drv.vchar_mutexlock);
				

			// --------------------------- semaphore method			
			/*			
			down(&vchar_drv.vchar_semaphore);
			vchar_drv.critical_resource = 0;
			up(&vchar_drv.vchar_semaphore);		
			*/			
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
	.mmap		= vchar_driver_mmap,
	.unlocked_ioctl = vchar_driver_ioctl
};



/* ***************************** entry points functions for file in procfs ***************************** */
static void* vchar_seq_start(struct seq_file *s, loff_t *pos) {
	char *msg;
	msg = kmalloc(256, GFP_KERNEL);
	// msg = vmalloc(256);	
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
	// vfree(pData);
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



/* *************************************** handle, config kernel timer and send interrupt signal *************************************** */
/*
static void handle_timer(unsigned long data) {
	vchar_ktimer_data_t* pData = (vchar_ktimer_data_t*)data;
	if (!pData) {
		printk(KERN_ERR "Can not handle a NULL pointer\n");
		return;
	}

	//++pData->param1;
	//--pData->param2;
	//printk(KERN_INFO "[Kernel timer] Pairs of opposite natural numbers: %d & %d\n", pData->param1, pData->param2);

	asm("int $0x3B");	// send intr signal, IRQ line 11 ~~ vector 0x3B
	printk(KERN_INFO "[Top-half task] [CPU %d] interrupt counter: %d\n", smp_processor_id(), vchar_drv.intr_cnt);

	mod_timer(&vchar_drv.vchar_ktimer, jiffies + 10*HZ); // change kernel timer to continue to task again
}



static void config_timer(struct timer_list* ktimer) {
	static vchar_ktimer_data_t data;
	ktimer->expires = jiffies + 10 * HZ;
	ktimer->function = handle_timer;
	ktimer->data = (unsigned long)&data;
}
*/



/* *********************************************************** init driver *********************************************************** */
void init_obj_lookaside(void *obj) {
	//char *p = (char *)obj;
	memset(obj, 0, OBJ_LOOKASIDE_SIZE);
}



static int __init vchar_driver_init(void)
{
	int ret;
	unsigned char i;	

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
	// vchar_drv.vchar_hw = vmalloc(sizeof(vchar_dev_t));
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
		ret = -1;
		goto failed_create_proc;
	}

	/* Init, config, register kernel timer */
	/*
	init_timer(&vchar_drv.vchar_ktimer);
	config_timer(&vchar_drv.vchar_ktimer);
	add_timer(&vchar_drv.vchar_ktimer);
	*/

	/* register ISR */
	/*
	ret = request_irq(IRQ_NUMBER, vchar_hw_isr, IRQF_SHARED, "vchar_dev", (void*)&vchar_drv.vcdev);
	if (ret) {
		printk(KERN_ERR "Failed to register ISR\n");
		goto failed_create_proc;
	}
	*/

	/* create tasklet dynamically for bottom-half task */
	/*
	vchar_drv.vchar_dynamic_tasklet = kzalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
	if (!vchar_drv.vchar_dynamic_tasklet) {
		printk(KERN_ERR "Failed to allocate memory for tasklet\n");
		ret = -ENOMEM;		
		goto failed_create_tasklet;
	}
	tasklet_init(vchar_drv.vchar_dynamic_tasklet, vchar_hw_bh_task, (unsigned long)&vchar_drv.intr_cnt);
	*/
	
	/* create user-defined workqueue */
	/*
	vchar_drv.vchar_user_workqueue = create_workqueue("vchar_workqueue");
	if (!vchar_drv.vchar_user_workqueue) {
		printk(KERN_ERR "Failed to create workqueue\n");
		ret = -1;		
		goto failed_create_workqueue;
	}
	*/

	/* init spinlock */
	// spin_lock_init(&vchar_drv.vchar_spinlock);

	/* init mutex lock */
	mutex_init(&vchar_drv.vchar_mutexlock);	
	
	/* init semaphore */
	// sema_init(&vchar_drv.vchar_semaphore, 1);	

	/* init lookaside cache */
	vchar_drv.vchar_lookaside_cache = kmem_cache_create("vchar_lookaside_cache", OBJ_LOOKASIDE_SIZE, 0, SLAB_PANIC, init_obj_lookaside);
	if (!vchar_drv.vchar_lookaside_cache) {
		printk(KERN_ERR "Failed to allocate memory for lookaside cache\n");
		ret = -ENOMEM;		
		goto failed_create_lookaside;
	}

	/* request kernel to use one of port regions */
	vchar_drv.vchar_ioport = request_region(VCHAR_IOPORT_BASE, VCHAR_IOPORT_LENGTH, VCHAR_IOPORT_NAME);
	if (vchar_drv.vchar_ioport)
		for (i = 0; i < VCHAR_IOPORT_LENGTH; i++)
			printk(KERN_INFO "Read data at 0x%02x address: 0x%02x\n", VCHAR_IOPORT(i), inb(VCHAR_IOPORT(i)));

	printk(KERN_INFO "Initialize virtual character driver successfully\n");
	return 0;

//failed_create_workqueue:
//failed_create_tasklet:
	//free_irq(IRQ_NUMBER, &vchar_drv.vcdev);
failed_create_lookaside:
	cdev_del(vchar_drv.vcdev);
failed_create_proc:
failed_alloc_cdev:
	vchar_hw_exit(vchar_drv.vchar_hw);
failed_init_hw:
	kfree(vchar_drv.vchar_hw);
	// vfree(vchar_drv.vchar_hw);
failed_alloc_struct:
	device_destroy(vchar_drv.dev_class, vchar_drv.dev_num);
failed_create_device:
	class_destroy(vchar_drv.dev_class);
failed_create_class:
	unregister_chrdev_region(vchar_drv.dev_num, 1);
failed_register_device_number:
	return ret;	
}



/* ************************************************* exit driver ************************************************* */
static void __exit vchar_driver_exit(void)
{
	/* release IO port region */
	if (vchar_drv.vchar_ioport)
		release_region(VCHAR_IOPORT_BASE, VCHAR_IOPORT_LENGTH);

	/* remove lookaside cache */
	kmem_cache_destroy(vchar_drv.vchar_lookaside_cache);

	/* unregister ISR */
	//tasklet_kill(&vchar_static_tasklet);
	//tasklet_kill(vchar_drv.vchar_dynamic_tasklet);
	/*
	cancel_work_sync(&vchar_static_work);
	cancel_delayed_work_sync(&vchar_static_delayed_work);
	destroy_workqueue(vchar_drv.vchar_user_workqueue);

	free_irq(IRQ_NUMBER, &vchar_drv.vcdev);
	*/

	/* remove kernel timer */
	//del_timer(&vchar_drv.vchar_ktimer);
	
	/* remove file /proc/vchar_proc */
	remove_proc_entry("vchar_proc", NULL);

	/* unregister entry point with kernel */
	cdev_del(vchar_drv.vcdev);

	/* release device */
	vchar_hw_exit(vchar_drv.vchar_hw);

	/* release memory of struct of the driver */
	// kfree(vchar_drv.vchar_hw);
	vfree(vchar_drv.vchar_hw);

	/* delete device file */
	device_destroy(vchar_drv.dev_class, vchar_drv.dev_num);
	class_destroy(vchar_drv.dev_class);

	/* release device number */
	unregister_chrdev_region(vchar_drv.dev_num, 1);

	printk(KERN_INFO "Exit virtual character driver\n");
}
/* ****************************************************************************************** OS specific - END ***************************************************************************************** */



module_init(vchar_driver_init);
module_exit(vchar_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_SUPPORTED_DEVICE("testdevice");
