#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/list.h>
#include<linux/slab.h>

struct sample
{
    int x;
    int y;
    struct list_head node;
};

LIST_HEAD(mylist);

int __init list_demo_init(void)
{
    int i;
    struct sample *ps;
    for(i = 1; i <= 10; i ++)
    {
        ps = kmalloc(sizeof(struct sample), GFP_KERNEL);
        ps->x = i * 100;
        ps->y = i * 100 + 10;
        list_add_tail(&ps->node, &mylist);
    }
    printk("Hello from list_demo\n");
    return 0;
}

void printdata(void){
    struct list_head* pcur;
    struct sample *ps;
    list_for_each(pcur, &mylist)
    {
        ps = list_entry(pcur, struct sample, node);
        printk("x is %d and y is %d\n", ps->x, ps->y);
    }
}

void __exit list_demo_exit(void){
    struct list_head *pcur, *pbak;
    struct sample *ps;
    
    printdata();

    list_for_each_safe(pcur, pbak, &mylist)
    {
        ps = list_entry(pcur, struct sample, node);
        kfree(ps);
    }
    printk("Leaving list_demo module\n");
}

module_init(list_demo_init);
module_exit(list_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nikhil");