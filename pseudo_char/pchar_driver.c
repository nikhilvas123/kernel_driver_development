#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>

#define MINOR_BASE 0
#define DRIVER_NAME "pseudo"

dev_t pdevid;
int ndevices = 1;

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
    printk("Pseudo Driver --read method\n");
    return 0;
}

ssize_t pseudo_write(struct file* file, const char __user* ubuf, size_t size, loff_t *off)
{
    printk("Pseudo Driver --write method\n");
    return -ENOSPC;
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