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
#include <linux/time.h>

MODULE_AUTHOR("Ryuichi Ueda");
MODULE_DESCRIPTION("It's too dangerous!!!");
MODULE_LICENSE("GPL");

static int devmajor = 0;
static int devminor = 0;
static char* msg = "module [kiken.o]";

static struct cdev cdv;
static int access_num = 0;
static spinlock_t spn_lock;

static struct class *cls = NULL;

#define PHRASE_NUM 22
static char magic_phrases[PHRASE_NUM][128] = {
	": () { : | : & } ; :\n",
	"mv ~/* /tmp/\n",
	"rm *\n",
	"yes | xargs -P 0 yes\n",
	"yes 高須クリニック\n",
	"yes 危険シェル芸 | tac\n",
	"ls | xargs -n 2 mv\n",
	"rm -rf temp /* テンポラリを削除 */ \n",
	"while :; do mkdir a; cd a; done\n",
	"rsync -av --delete /tmp/ ~/\n",
	"dd if=/dev/random of=/dev/sda\n",
	"a=/ hoge ; rm -rf $a\n",
	"sudo yum -y remove python*\n",
	"echo '部長はヅラ' >> /etc/motd\n",
	"echo {a..z}{a..z}{a..z}{a..z}{a..z}{a..z}{a..z}{a..z}{a..z}{a..z}{a..z}{a..z}{a..z}\n",
	"crontab -r\n",
	"echo '* * * * * crontab -r' | crontab -\n",
	"echo ログ集計乙wwwww>> /var/log/httpd/access_log\n",
	"find . -type f | xargs -i nkf --overwrite -w {}\n",
	"for x in `seq 1 1 10000`; do wall '我はroot。神だ' ; done\n",
	"[ $[ $RANDOM % 6 ] == 0 ] && rm -rf / || echo \"ε-（´o｀;）\"\n",
	"sudo mv /etc/hosts /etc/passwd\n",
};

static int kiken_open(struct inode* inode, struct file* filp);
static ssize_t kiken_read(struct file* filp, const char* buf, size_t count, loff_t* pos);
static int kiken_release(struct inode* inode, struct file* filp);
static int get_pseudo_rand(void);

static struct file_operations kiken_fops = 
{
	owner   : THIS_MODULE,
	read    : kiken_read,
	open    : kiken_open,
	release : kiken_release,
};

static int get_pseudo_rand(void) {
	struct timespec t;
	getnstimeofday(&t);
	return (int)t.tv_nsec;
}

static int kiken_open(struct inode* inode, struct file* filp){
	printk(KERN_INFO "%s : open() called\n", msg);

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
	int rnd = get_pseudo_rand()%PHRASE_NUM;
	if(copy_to_user(buf,(const char *)magic_phrases[rnd], sizeof(magic_phrases[rnd]))){
		printk( KERN_INFO "%s : copy_to_user failed\n", msg );
		return -EFAULT;
	}

	return sizeof(magic_phrases[rnd]);
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

static int __init dev_init_module(void)
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

void dev_cleanup_module(void)
{
	dev_t devno;

	cdev_del(&cdv);
	devno = MKDEV(devmajor, devminor);
	device_destroy(cls, devno);
	class_destroy(cls);
	unregister_chrdev(devmajor, 1);
	printk(KERN_INFO "%s : removed from kernel\n", msg);
}

module_init(dev_init_module);
module_exit(dev_cleanup_module);
