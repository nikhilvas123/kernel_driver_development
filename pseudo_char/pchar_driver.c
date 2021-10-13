#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>

#define MINOR_BASE 0
#define DRIVER_NAME "pseudo"
#define MAX_BUF_SIZE 1024

dev_t pdevid;

unsigned char *pbuffer;

int ndevices = 1;
int rd_offset = 0;  /* head */
int wr_offset = 0;  /* tail */
int buflen = 0;

struct kfifo myfifo;
struct cdev pseudo_cdev;
struct device* pdev;
struct class* pclass;

typedef struct prev_obj
{
    struct cdev pseudo_cdev;
    struct kfifo myfifo;
    struct device* pdev;
    struct class* pclass;
}PREV_OBJ;

PREV_OBJ* pobj;

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
    char *tempbuf;
    printk("Pseudo Driver --read method\n");
    if(kfifo_is_empty(&myfifo))
    {
        printk("Buffer is empty\n");
        return 0;
    }
    rcount = size;
    
    if(rcount > kfifo_len(&myfifo))
        rcount = kfifo_len(&myfifo);
    
    tempbuf = kmalloc(rcount, MAX_BUF_SIZE);
    kfifo_out(&myfifo, tempbuf, rcount);

    ret = copy_to_user(ubuf, tempbuf, rcount);
    
    if(ret){
        printk("Copy to user failed\n");
        return -EINVAL;
    }

    kfree(tempbuf);

    return rcount;
}

ssize_t pseudo_write(struct file* file, const char __user* ubuf, size_t size, loff_t *off)
{
    int wcount, ret;
    char *tempbuf;
    printk("Pseudo Driver --write method\n");

    if(kfifo_is_full(&myfifo))
    {
        printk("Buffer is full\n");
        return -ENOSPC;
    }
    wcount = size;
    if(wcount > kfifo_avail(&myfifo))
        wcount = kfifo_avail(&myfifo);
    
    tempbuf = kmalloc(wcount, GFP_KERNEL);

    ret = copy_from_user(tempbuf, ubuf, wcount);
    if(ret)
    {
        printk("Copy from user failed\n");
        return -EINVAL;
    }
    kfifo_in(&myfifo, tempbuf, wcount);
    kfree(tempbuf);
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
    
    pobj = kmalloc(sizeof(PREV_OBJ), GFP_KERNEL);

    pobj->pclass = class_create(THIS_MODULE, DRIVER_NAME);

    if(pobj->pclass == NULL)
    {
        printk("Failed to create class");
        return -EINVAL;
    }

    ret = alloc_chrdev_region(&pdevid, MINOR_BASE, ndevices, DRIVER_NAME);
    
    if(ret != 0){
        printk("Pseudo: failed to register driver\n");
        return -EINVAL;
    }

    pbuffer = kmalloc(MAX_BUF_SIZE, GFP_KERNEL);

    if(pbuffer == NULL)
    {
        printk("Unable to allocate memory");
        return -ENOMEM;
    }
    kfifo_init(&(pobj->myfifo), pbuffer, MAX_BUF_SIZE);

    if(pbuffer == NULL)
    {
        printk("Unable to allocate memory\n");
        return -ENOMEM;
    }


    cdev_init(&(pobj->pseudo_cdev), &fops);
    kobject_set_name(&(pobj->pseudo_cdev.kobj), "psample%d", i);
    cdev_add(&(pobj->pseudo_cdev), pdevid, 1);

    pobj->pdev = device_create(pobj->pclass, NULL, pdevid, NULL, "psample%d", i);

    if(pobj->pdev == NULL)
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
    kfifo_free(&myfifo);
    // kfree(pbuffer);
    device_destroy(pclass, pdevid);
    cdev_del(&pseudo_cdev);
    unregister_chrdev_region(pdevid, ndevices);
    class_destroy(pclass);
    kfree(pobj);
    printk("Psuedo: Driver unregistered\n");
}

module_init(pseudo_init);
module_exit(pseudo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nikhil");
MODULE_DESCRIPTION("Pseudo character driver");