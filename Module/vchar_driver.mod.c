#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x28950ef1, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x999e8297, __VMLINUX_SYMBOL_STR(vfree) },
	{ 0xa16aae11, __VMLINUX_SYMBOL_STR(remove_proc_entry) },
	{ 0xacfa5975, __VMLINUX_SYMBOL_STR(kmem_cache_destroy) },
	{ 0x7c61340c, __VMLINUX_SYMBOL_STR(__release_region) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0x450c190, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0xacbfe419, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x1fedf0f4, __VMLINUX_SYMBOL_STR(__request_region) },
	{ 0xff7559e4, __VMLINUX_SYMBOL_STR(ioport_resource) },
	{ 0x3fa89e8f, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0xaf5517a9, __VMLINUX_SYMBOL_STR(kmem_cache_create) },
	{ 0x9a025cd5, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x8c34c149, __VMLINUX_SYMBOL_STR(proc_create_data) },
	{ 0x5f675a65, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0xdacd8618, __VMLINUX_SYMBOL_STR(cdev_init) },
	{ 0x798d5ca6, __VMLINUX_SYMBOL_STR(cdev_alloc) },
	{ 0x196103b4, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x7e5df8e3, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0x29537c9e, __VMLINUX_SYMBOL_STR(alloc_chrdev_region) },
	{ 0x4ed12f73, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x9abdea30, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0x77e2f33, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x4f68e5c9, __VMLINUX_SYMBOL_STR(do_gettimeofday) },
	{ 0x19ee3d71, __VMLINUX_SYMBOL_STR(kmem_cache_free) },
	{ 0x71de9b3f, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x88db9f48, __VMLINUX_SYMBOL_STR(__check_object_size) },
	{ 0xd62c833f, __VMLINUX_SYMBOL_STR(schedule_timeout) },
	{ 0xb8c7ff88, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0xd11b7a3e, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0x34184afe, __VMLINUX_SYMBOL_STR(current_kernel_time) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0x4302d0eb, __VMLINUX_SYMBOL_STR(free_pages) },
	{ 0x9b388444, __VMLINUX_SYMBOL_STR(get_zeroed_page) },
	{ 0xe84cb310, __VMLINUX_SYMBOL_STR(remap_pfn_range) },
	{ 0x5944d015, __VMLINUX_SYMBOL_STR(__cachemode2pte_tbl) },
	{ 0x45449b56, __VMLINUX_SYMBOL_STR(boot_cpu_data) },
	{ 0x7cd8d75e, __VMLINUX_SYMBOL_STR(page_offset_base) },
	{ 0x4c9d28b0, __VMLINUX_SYMBOL_STR(phys_base) },
	{ 0xf0fdf6cb, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x9a1fc4b4, __VMLINUX_SYMBOL_STR(jiffies_to_timeval) },
	{ 0x9c3df9b4, __VMLINUX_SYMBOL_STR(seq_read) },
	{ 0x41ec4c1a, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x98ab5c8d, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x74df1d4, __VMLINUX_SYMBOL_STR(seq_printf) },
	{ 0x257f3887, __VMLINUX_SYMBOL_STR(seq_open) },
	{ 0x4f897611, __VMLINUX_SYMBOL_STR(seq_release) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "CC1DB9FC62228E63D539670");
MODULE_INFO(rhelversion, "7.9");
#ifdef RETPOLINE
	MODULE_INFO(retpoline, "Y");
#endif
#ifdef CONFIG_MPROFILE_KERNEL
	MODULE_INFO(mprofile, "Y");
#endif
