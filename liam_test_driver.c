#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/uaccess.h>      /* copy_*_user */
#include <linux/slab.h>

MODULE_AUTHOR("Liam Zhang");
MODULE_LICENSE("Dual BSD/GPL");   

void *ker_buf = NULL;
unsigned int ker_buf_len = 0;
static dev_t liam_dev_num;
static struct cdev liam_dev;  

static const char    g_s_Hello_World_string[] = "Hello world from kernel mode!\n\0";
static const ssize_t g_s_Hello_World_size = sizeof(g_s_Hello_World_string);

static int copen(struct inode *node, struct file *filep)
{
    printk(KERN_ALERT "led_dev is open!\n");        
    return 0;
} 


static ssize_t cread (struct file *filep, char __user *buf, size_t count, loff_t *position)
{        
    printk(KERN_ALERT "read, count is %ld\n", count);
    if(ker_buf != NULL)
    {
        if( *position >= ker_buf_len)
            return 0;
        if( *position + count > ker_buf_len )
            count = ker_buf_len - *position;
        if(copy_to_user(buf, ker_buf + *position, count) != 0 )
            return -EFAULT;
        *position += count;
        return count;
    }
    /* If position is behind the end of a file we have nothing to read */
    if( *position >= g_s_Hello_World_size )
        return 0;
    /* If a user tries to read more than we have, read only as many bytes as we have */
    if( *position + count > g_s_Hello_World_size )
        count = g_s_Hello_World_size - *position;
    if( copy_to_user(buf, g_s_Hello_World_string + *position, count) != 0 )
        return -EFAULT;    
    /* Move reading position */
    *position += count;
    return count;
} 


static ssize_t cwrite (struct file *filep, const char __user *buf, size_t count, loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    printk(KERN_ALERT "led_dev is write: %ld\n", count);
    ker_buf_len = count;
    ker_buf = kmalloc(count, GFP_KERNEL);
    if (copy_from_user(ker_buf, buf, count)) 
    {
        retval = -EFAULT;
    }
    else
    {
       printk(KERN_ALERT "The str from user-space: %s",ker_buf);
       retval = count;
    }
    return retval;
} 
 
static struct file_operations liam_dev_fops =
{
    .read  = cread,
    .write = cwrite,
    .open  = copen,
    .owner = THIS_MODULE
};


static int liam_init(void)
{
    int res;
    printk(KERN_ALERT "Hello world\n");
    res = alloc_chrdev_region(&liam_dev_num, 0, 1, "liam_dev");
    if (res < 0)
    {
        printk(KERN_WARNING "can't get major %d\n", MAJOR(liam_dev_num));
        return res;
    }
    printk(KERN_ALERT "The device major number: %d\n", MAJOR(liam_dev_num));
    cdev_init (&liam_dev, &liam_dev_fops); 
    cdev_add (&liam_dev, liam_dev_num, 1); 
    
    return 0;
}
   
static void liam_exit(void)
{
    cdev_del (&liam_dev);
    unregister_chrdev_region(liam_dev_num, 1);
    kfree(ker_buf);
    printk(KERN_ALERT "Goodbye\n");
}
   
module_init(liam_init);
module_exit(liam_exit);
