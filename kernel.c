#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>

#define RED_LED 17
#define YELLOW_LED 27

static struct proc_dir_entry *proc_file;
static int current_temp = 0;

static ssize_t tempmon_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
    char msg[64];
    int msg_len;

    if (*offset > 0)
        return 0;

    msg_len = sprintf(msg, "Current temperature is %d\n", current_temp);
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
    current_temp = temp_value;

    if (current_temp > 30) {
        gpio_set_value(RED_LED, 1);
        gpio_set_value(YELLOW_LED, 0);
    } else if (current_temp < 15) {
        gpio_set_value(RED_LED, 0);
        gpio_set_value(YELLOW_LED, 1);
    } else {
        gpio_set_value(RED_LED, 0);
        gpio_set_value(YELLOW_LED, 0);
    }

    return len;
}

static const struct proc_ops proc_file_ops = {
    .proc_read = tempmon_read,
    .proc_write = tempmon_write,
};

static int __init tempmon_start(void)
{
    gpio_request(RED_LED, "red_led");
    gpio_direction_output(RED_LED, 0);

    gpio_request(YELLOW_LED, "yellow_led");
    gpio_direction_output(YELLOW_LED, 0);

    proc_file = proc_create("temp_monitor", 0666, NULL, &proc_file_ops);
    if (!proc_file)
        return -ENOMEM;

    printk(KERN_INFO "tempmon: module started\n");
    return 0;
}

static void __exit tempmon_end(void)
{
    proc_remove(proc_file);
    gpio_set_value(RED_LED, 0);
    gpio_set_value(YELLOW_LED, 0);
    gpio_free(RED_LED);
    gpio_free(YELLOW_LED);
    printk(KERN_INFO "tempmon: module stopped\n");
}

module_init(tempmon_start);
module_exit(tempmon_end);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ch1ru");
MODULE_DESCRIPTION("Simple temperature monitor module for Raspberry Pi");
