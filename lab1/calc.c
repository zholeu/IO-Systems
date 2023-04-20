#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h> 
#include <linux/proc_fs.h>

#define DEVICE_NAME "var2"
#define PROC_NAME "var2"


static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static struct proc_dir_entry *proc_entry;
static char proc_buf[256];
static int proc_buf_size = 0;
#define BUF_SIZE 20
static int buffer[BUF_SIZE] = {0};
static int buf_index = 0;

static int perform_operation(char *operation)
{
    int a, b;
    char op;

    if (sscanf(operation, "%d%c%d", &a, &op, &b) != 3)
        return 0;

    switch (op)
    {
        case '+':
            return a + b;
        case '-':
            return a - b;
        case '*':
            return a * b;
        case '/':
            if (b == 0){
                printk(KERN_ERR "Cannot divide by zero\n");
                return -EINVAL;
            }   
            else
                return a / b;
        default:
            return 0;
    }
}


static ssize_t var2_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    int i, pos = 0;
    char tmp[256];
    ssize_t ret;

    for (i = buf_index - 1; i >= 0 && pos < len; i--)
    {
        pos += sprintf(tmp + pos, "%d, ", buffer[i]);

    }
    if (pos == 0)
        return 0;
    if (!filp->private_data) {
        printk(KERN_INFO "BUFFER'S INTERNALS: \n");
        for (i = buf_index - 1; i >= 0; i--)
        {
            printk(KERN_INFO "%d: %d\n", i + 1, buffer[i]);
        }
        filp->private_data = (void*)1; 
    }

    if (pos > len)
        return -EINVAL;
    tmp[pos - 2] = '\n'; 
    tmp[pos - 1] = '\0'; 
    ret = simple_read_from_buffer(buf, len, off, tmp, pos);
    if (ret < 0)
        return ret;

    return ret;
}

static ssize_t var2_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    char operation[256];
    int result;

    if (len >= sizeof(operation))
        return -EINVAL;

    if (raw_copy_from_user(operation, buf, len))
        return -EFAULT;

    operation[len - 1] = '\0'; 

    result = perform_operation(operation);
    if (result != -EINVAL) {

    if (buf_index < BUF_SIZE)
    {
        buffer[buf_index++] = result;
    }
    else
    {
        memmove(buffer, buffer + 1, (BUF_SIZE - 1) * sizeof(int));
        buffer[BUF_SIZE - 1] = result;
    }
    
    }
    proc_buf_size = snprintf(proc_buf, sizeof(proc_buf), "%d\n", result);

    return len;
}

static const struct file_operations var2_fops = {
    .owner = THIS_MODULE,
    .read = var2_read,
    .write = var2_write,
};

static ssize_t var2_proc_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    int i, pos = 0;
    char tmp[32];

    for (i = buf_index - 1; i >= 0 && pos < len; i--)
    {
        pos += snprintf(tmp + pos, sizeof(tmp) - pos, "%d\n", buffer[i]);
    }
    if (pos == 0)
        return 0;
    if (pos > len) 
        return -EINVAL;

    return simple_read_from_buffer(buf, len, off, tmp, pos);
}


static const struct file_operations var2_proc_fops = {
    .owner = THIS_MODULE,
    .read = var2_proc_read,
};

static int __init var2_init(void)
{
    int ret = 0;

    if ((ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME)) < 0)
{
    printk(KERN_ERR "Failed to allocate character device numbers\n");
    return ret;
}

cdev_init(&c_dev, &var2_fops);
c_dev.owner = THIS_MODULE;

if ((ret = cdev_add(&c_dev, dev, 1)) < 0)
{
    printk(KERN_ERR "Failed to add character device\n");
    goto fail_cdev_add;
}

if (IS_ERR(cl = class_create(THIS_MODULE, DEVICE_NAME)))
{
    printk(KERN_ERR "Failed to create device class\n");
    goto fail_class_create;
}

if (IS_ERR(device_create(cl, NULL, dev, NULL, DEVICE_NAME)))
{
    printk(KERN_ERR "Failed to create character device file\n");
    goto fail_device_create;
}

proc_entry = proc_create(PROC_NAME, 0644, NULL, &var2_proc_fops);
if (proc_entry == NULL)
{
    printk(KERN_ERR "Failed to create proc file system entry\n");
    goto fail_proc_create;
}

printk(KERN_INFO "var2 driver loaded\n");
return 0;
fail_proc_create:
device_destroy(cl, dev);
fail_device_create:
class_destroy(cl);
fail_class_create:
cdev_del(&c_dev);
fail_cdev_add:
unregister_chrdev_region(dev, 1);
return ret;
}

static void __exit var2_exit(void)
{
remove_proc_entry(PROC_NAME, NULL);
device_destroy(cl, dev);
cdev_del(&c_dev);
class_destroy(cl);
unregister_chrdev_region(dev, 1);
printk(KERN_INFO "var2 driver unloaded\n");
}

module_init(var2_init);
module_exit(var2_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zholeu");
MODULE_DESCRIPTION("A simple calculator module that allows basic arithmetic operations such as +, -, *, and /. The module takes input from the user via a command-line interface and provides the result of the operation as output. It also includes a history feature that allows the user to view their previous calculations.");


