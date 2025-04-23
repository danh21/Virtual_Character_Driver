#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0xe16cab4a, "boot_cpu_data" },
	{ 0x98378a1d, "cc_mkdec" },
	{ 0x8ef00bb, "remap_pfn_range" },
	{ 0x46cf10eb, "cachemode2protval" },
	{ 0x4c9d28b0, "phys_base" },
	{ 0xdbdf6c92, "ioport_resource" },
	{ 0x1035c7c2, "__release_region" },
	{ 0xddf6706a, "kmem_cache_destroy" },
	{ 0x3addb2e7, "remove_proc_entry" },
	{ 0x39d2b405, "cdev_del" },
	{ 0x4302d0eb, "free_pages" },
	{ 0x999e8297, "vfree" },
	{ 0x38655e10, "device_destroy" },
	{ 0x288176f5, "class_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xf09b5d9a, "get_zeroed_page" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x169675e2, "class_create" },
	{ 0x92649ec3, "device_create" },
	{ 0x7de5a4fa, "cdev_alloc" },
	{ 0x14777360, "cdev_init" },
	{ 0xaa5419b1, "cdev_add" },
	{ 0x4f4ab2c1, "proc_create" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0xa9fc5eda, "kmem_cache_create" },
	{ 0x85bd1608, "__request_region" },
	{ 0x69acdf38, "memcpy" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x2b6e2cf, "pcpu_hot" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xf9a482f9, "msleep" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x122c3a7e, "_printk" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x15ba50a6, "jiffies" },
	{ 0x37a0cba, "kfree" },
	{ 0x7be0a3c7, "seq_release" },
	{ 0x79ccc597, "seq_read" },
	{ 0xfddf66dc, "seq_open" },
	{ 0x2f9b7e16, "seq_printf" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x4c03a563, "random_kmalloc_seed" },
	{ 0x24980310, "kmalloc_caches" },
	{ 0x1d199deb, "kmalloc_trace" },
	{ 0x188ea314, "jiffies_to_timespec64" },
	{ 0x6ad2b3e, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "F4489504F79D440DDE9A875");
