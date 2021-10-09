#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define MINOR_BASE 0
#define DRIVER_NAME "pseudo"
#define MAX_BUF_SIZE 1024

dev_t pdevid;
int ndevices = 1;

unsigned char *pbuffer;
int rd_offset = 0;  /* head */
int wr_offset = 0;  /* tail */
int buflen = 0;

struct cdev pseudo_cdev;
struct device *pdev;
struct class *pclass;

int pseudo_open(struct inode* inode, struct file* file)
{
    printk("Psuedo Driver --open method\n");
    return 0;
}  

int pseudo_close(struct inode* inode, struct file* file)
{
    printk("Psuedo Driver --close method\n");
    return 0;
}  

ssize_t pseudo_read(struct file* file, char __user* ubuf, size_t size, loff_t* off)
{   
    int ret, rcount;

    printk("Pseudo Driver --read method\n");
    if(buflen == 0)
    {
        printk("Buffer is empty\n");
        return 0;
    }
    rcount = size;
    
    if(rcount > buflen)
        rcount = buflen;
    
    ret = copy_to_user(ubuf, pbuffer + rd_offset, rcount);
    if(ret){
        printk("Copy to user failed\n");
        return -EINVAL;
    }

    rd_offset += rcount;
    buflen -= rcount;
    return rcount;

    return 0;
}

ssize_t pseudo_write(struct file* file, const char __user* ubuf, size_t size, loff_t *off)
{
    int wcount, ret;
    printk("Pseudo Driver --write method\n");

    if(wr_offset >= MAX_BUF_SIZE)
    {
        printk("Buffer is full\n");
        return -ENOSPC;
    }
    wcount = size;
    if(wcount > MAX_BUF_SIZE - wr_offset)
        wcount = MAX_BUF_SIZE - wr_offset;
    
    ret = copy_from_user(pbuffer, ubuf, wcount);
    if(ret)
    {
        printk("Copy from user failed\n");
        return -EINVAL;
    }
    wr_offset += wcount;
    buflen += wcount;
    printk("Successfully written %d bytes\n", wcount);
    return wcount;
}

struct file_operations fops = {
    .open = pseudo_open,
    .read = pseudo_read,
    .write = pseudo_write,
    .release = pseudo_close
};

static int __init pseudo_init(void)
{
    int ret;
    int i = 0;
    
    pclass = class_create(THIS_MODULE, DRIVER_NAME);

    ret = alloc_chrdev_region(&pdevid, MINOR_BASE, ndevices, DRIVER_NAME);
    
    if(ret != 0){
        printk("Pseudo: failed to register driver\n");
        return -EINVAL;
    }

    pbuffer = kmalloc(MAX_BUF_SIZE, GFP_KERNEL);

    if(pbuffer == NULL)
    {
        printk("Unable to allocate memory\n");
        return -ENOMEM;
    }


    cdev_init(&pseudo_cdev, &fops);
    kobject_set_name(&pseudo_cdev.kobj, "psample%d", i);
    cdev_add(&pseudo_cdev, pdevid, 1);

    pdev = device_create(pclass, NULL, pdevid, NULL, "psample%d", i);

    if(pdev == NULL)
    {
        printk("Failed to create device");
        return -EINVAL;
    }


    printk("Successfully registered, major = %d, minor = %d\n", MAJOR(pdevid), MINOR(pdevid));
    printk("Pseudo driver sample welcome\n");
    
    return 0;
}

static void __exit pseudo_exit(void)
{
    kfree(pbuffer);
    device_destroy(pclass, pdevid);
    cdev_del(&pseudo_cdev);
    unregister_chrdev_region(pdevid, ndevices);
    class_destroy(pclass);
    printk("Psuedo: Driver unregistered\n");
}

module_init(pseudo_init);
module_exit(pseudo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nikhil");
MODULE_DESCRIPTION("Pseudo character driver");