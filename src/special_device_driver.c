#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>       /* printk() */
#include <linux/slab.h>         /* kmalloc() */
#include <linux/fs.h>           /* everything... */
#include <linux/errno.h>        /* error codes */
#include <linux/types.h>        /* size_t */
#include <linux/cdev.h>         /* char device */
#include <linux/semaphore.h>    /* semaphore for mutual exclusion */
#include <linux/uaccess.h>      /* copy_from_user */
#include <linux/kthread.h>      /* for kernel threads */
#include <linux/delay.h>        /* msleep() for wasting time */

#define DRIVER_PATH "/dev/special_driver"  /* Device file path */

/* Parameters */
int special_major = 0;
int special_minor = 0;
int memsize = 255;

module_param(special_major, int, S_IRUGO);
module_param(special_minor, int, S_IRUGO);
module_param(memsize, int, S_IRUGO);

MODULE_AUTHOR("Your Name");
MODULE_LICENSE("Dual BSD/GPL");

/* Special device structure */
struct special_dev {
    char *data;         /* Pointer to data area */
    int memsize;        /* Memory size for data */
    struct semaphore sem;   /* Semaphore for mutual exclusion */
    struct cdev cdev;   /* Character device structure */
};

struct special_dev special_device;

/* Open the device */
int special_open(struct inode *inode, struct file *filp) {
    struct special_dev *dev = container_of(inode->i_cdev, struct special_dev, cdev);
    filp->private_data = dev;  /* Store device pointer in file structure */
    return 0;
}

/* Release the device */
int special_release(struct inode *inode, struct file *filp) {
    return 0;
}

/* Write system call: Log the data (thread identifier) to the kernel log */
ssize_t special_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    struct special_dev *dev = filp->private_data;
    ssize_t retval = 0;

    /* Ensure mutual exclusion using semaphore */
    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    /* Limit count to the maximum size */
    if (count >= dev->memsize)
        count = dev->memsize;

    /* Copy data from user-space to kernel-space */
    if (copy_from_user(dev->data, buf, count)) {
        retval = -EFAULT;
        goto out;
    }

    /* Log the received data (Thread identifier) to kernel log */
    printk("%s", dev->data);  /* Logs "[1" to the kernel log */

    retval = count;

out:
    up(&dev->sem);
    return retval;
}

/* File operations for the special device */
struct file_operations special_fops = {
    .owner =    THIS_MODULE,
    .write =    special_write,
    .open =     special_open,
    .release =  special_release,
};

/* Cleanup the module */
void special_cleanup_module(void) {
    dev_t devno = MKDEV(special_major, special_minor);

    /* Free the cdev entries */
    cdev_del(&special_device.cdev);

    /* Free the memory */
    kfree(special_device.data);

    /* Unregister the device */
    unregister_chrdev_region(devno, 1);
}

/* Initialize the module */
int special_init_module(void) {
    int result, err;
    dev_t dev = 0;

    /* Register the device (major number) */
    if (special_major) {  
        dev = MKDEV(special_major, special_minor);
        result = register_chrdev_region(dev, 1, "special");
    } else {            
        result = alloc_chrdev_region(&dev, special_minor, 1, "special");
        special_major = MAJOR(dev);
    }

    if (result < 0) {
        printk(KERN_WARNING "special: can't get major %d\n", special_major);
        return result;
    }

    /* Allocate memory for the device */
    special_device.memsize = memsize;
    special_device.data = kmalloc(memsize * sizeof(char), GFP_KERNEL);
    memset(special_device.data, 0, memsize * sizeof(char));

    /* Initialize the semaphore for mutual exclusion */
    sema_init(&special_device.sem, 1);

    /* Initialize the cdev structure */
    cdev_init(&special_device.cdev, &special_fops);
    special_device.cdev.owner = THIS_MODULE;
    special_device.cdev.ops = &special_fops;
    err = cdev_add(&special_device.cdev, dev, 1);

    if (err)  
        printk(KERN_NOTICE "Error %d adding special", err);
    else
        printk(KERN_NOTICE "Special Added major: %d minor: %d", special_major, special_minor);

    return 0;
}

module_init(special_init_module);
module_exit(special_cleanup_module);
