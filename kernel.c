#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/timekeeping.h>

static struct proc_dir_entry *proc_file;
static int current_temp = 0;
static u64 last_update = 0;

static ssize_t tempmon_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
    char msg[128];
    int msg_len;

    if (*offset > 0)
        return 0;

    msg_len = sprintf(msg, 
        "Current temperature: %d\nLast update: %llu seconds since epoch\n",
        current_temp, last_update);
    if (copy_to_user(buffer, msg, msg_len))
        return -EFAULT;

    *offset = msg_len;
    return msg_len;
}

static ssize_t tempmon_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
    char buf[16];
    int temp_value;

    if (len > 15)
        len = 15;

    if (copy_from_user(buf, buffer, len))
        return -EFAULT;

    buf[len] = '\0';
    sscanf(buf, "%d", &temp_value);

    // Validation: DS18B20 range -55°C to 125°C
    if (temp_value < -55 || temp_value > 125)
        return -EINVAL;
    current_temp = temp_value;

    last_update = ktime_get_real_seconds();

    return len;
}

static const struct proc_ops proc_file_ops = {
    .proc_read = tempmon_read,
    .proc_write = tempmon_write,
};

static int __init tempmon_start(void)
{

    proc_file = proc_create("temp_monitor", 0666, NULL, &proc_file_ops);
    if (!proc_file)
        return -ENOMEM;

    printk(KERN_INFO "tempmon: module started\n");
    return 0;
}

static void __exit tempmon_end(void)
{
    proc_remove(proc_file);
    printk(KERN_INFO "tempmon: module stopped\n");
}

module_init(tempmon_start);
module_exit(tempmon_end);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ch1ru");
MODULE_DESCRIPTION("Simple temperature monitor module for Raspberry Pi Zero");
