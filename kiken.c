/* kiken.c
 *
As shown in the code, this code can be distributed under GNU GPL.
*/
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_AUTHOR("Ryuichi Ueda");
MODULE_DESCRIPTION("It's too dangerous!!!");
MODULE_LICENSE("GPL");

static int devmajor = 0;
static int devminor = 0;
static char* devname  = "Kiken Shell Gei Generator";
static char* msg = "module [kiken.o]";

static struct cdev cdv;
static int access_num = 0;
static spinlock_t spn_lock;

static struct class *cls = NULL;

static int kiken_open(struct inode* inode, struct file* filp);
static ssize_t kiken_read(struct file* filp, const char* buf, size_t count, loff_t* pos);
//static ssize_t kiken_write(struct file* filp, const char* buf, size_t count, loff_t* pos);
static int kiken_release(struct inode* inode, struct file* filp);

static struct file_operations kiken_fops = 
{
	owner   : THIS_MODULE,
	read    : kiken_read,
	//write   : kiken_write,
	open    : kiken_open,
	release : kiken_release,
};

static int kiken_open(struct inode* inode, struct file* filp){
	printk(KERN_INFO "%s : open()  called\n", msg);

	spin_lock(&spn_lock);

	if(access_num){
		spin_unlock(&spn_lock);
		return -EBUSY;
	}

	access_num++;
	spin_unlock(&spn_lock);

	return 0;
}

static ssize_t kiken_read(struct file* filp, const char* buf, size_t count, loff_t* pos)
{
	const char aho[] = ": () { : | : & } ; :\n";
	copy_to_user(buf, aho, sizeof aho);

	printk(": () { : | : & } ; :\n");
	return sizeof aho;
}

/*
static ssize_t kiken_write(struct file* filp, const char* buf, size_t count, loff_t* pos)
{
	return 0;
}
*/

static int kiken_release(struct inode* inode, struct file* filp)
{
	printk(KERN_INFO "%s : close() called\n", msg);

	spin_lock(&spn_lock);
	access_num--;
	spin_unlock(&spn_lock);

	return 0;
}

int init_module(void)
{
	int retval;
	dev_t dev, devno;
	
	retval =  alloc_chrdev_region(&dev, 0, 1, "kiken");
	if(retval < 0){
		printk(KERN_ERR "alloc_chrdev_region failed.\n");
		return retval;
	}
	
	cls = class_create(THIS_MODULE,"kiken");
	if(IS_ERR(cls))
		return PTR_ERR(cls);

	devmajor = MAJOR(dev);
	devno = MKDEV(devmajor, devminor);
	cdev_init(&cdv, &kiken_fops);
	cdv.owner = THIS_MODULE;
	if(cdev_add(&cdv, devno, 1) < 0)
		printk(KERN_ERR "cdev_add failed minor = %d\n", devminor);
	else
		device_create(cls, NULL, devno, NULL, "kiken%u",devminor);

	return 0;
}

void cleanup_module(void)
{
	dev_t devno;

	cdev_del(&cdv);
	devno = MKDEV(devmajor, devminor);
	device_destroy(cls, devno);
	class_destroy(cls);
	unregister_chrdev(devmajor, devname);
	printk(KERN_INFO "%s : removed from kernel\n", msg);
}
