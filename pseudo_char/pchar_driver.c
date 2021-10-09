#include <linux/fs.h>
#define MINOR_BASE 0
#define DRIVER_NAME "pseudo"
dev_t pdevid;
int ndevices = 1;

static int __init pseudo_init(void)
{
    int ret;
    ret = alloc_chrdev_region(&pdevid, MINOR_BASE, ndevices, DRIVER_NAME);
    
    if(ret != 0){
        printk("Pseudo: failed to register driver\n");
        return -EINVAL;
    }
    printk("Successfully registered, major = %d, minor = %d\n", MAJOR(pdevid), MINOR(pdevid));
    printk("Pseudo driver sample welcome\n");
    
    return 0;
}

static void __exit pseudo_exit(void)
{
    unregister_chrdev_region(pdevid, ndevices);
    printk("Psuedo: Driver unregistered\n");
}