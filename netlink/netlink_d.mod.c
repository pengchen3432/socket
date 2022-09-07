#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

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
	{ 0xcb440b5e, "module_layout" },
	{ 0xb6cb78e1, "sock_release" },
	{ 0x903dca2f, "__netlink_kernel_create" },
	{ 0x6e06a909, "init_net" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x46c76d87, "kfree_skb" },
	{ 0x4a3ad70e, "wait_for_completion_timeout" },
	{ 0x608741b5, "__init_swait_queue_head" },
	{ 0x296695f, "refcount_warn_saturate" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x9a65940e, "netlink_unicast" },
	{ 0x4106ddac, "__nlmsg_put" },
	{ 0x487ed25d, "__alloc_skb" },
	{ 0x92997ed8, "_printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "EBE69DDC42721A3A7AA53EF");
