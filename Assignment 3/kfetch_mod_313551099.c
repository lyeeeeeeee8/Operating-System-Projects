#include <linux/atomic.h> 
#include <linux/cdev.h> 
#include <linux/delay.h> 
#include <linux/device.h> 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/kernel.h>
#include <linux/module.h> 
#include <linux/printk.h> 
#include <linux/types.h> 
#include <linux/uaccess.h>
#include <linux/version.h> 
#include <asm/errno.h> 
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/sched.h>
#include <linux/utsname.h>
#include <asm/page.h>
#include <linux/ktime.h>
#include <linux/sysinfo.h>
#include "kfetch.h"

#define DEVICE_NAME "kfetch" 
#define CLASS_NAME  "kfetch_class" 
#define BUFFER_SIZE 1024 
char logo[8][30] = {
    "        .-.           ",
    "       (.. |          ", 
    "       <>  |          ", 
    "      / --- \\         ", 
    "     ( |   | )        ", 
    "   |\\\\_)__(_//|       ",
    "  <__)------(__>      ",
    "                      "
};
enum { 
    CDEV_NOT_USED = 0, 
    CDEV_EXCLUSIVE_OPEN = 1, 
};

static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED); 
static DEFINE_MUTEX(kfetch_mutex);
static int major_number;              
static struct class* kfetch_class = NULL;  
static struct device* kfetch_device = NULL; 
static char info_buffer[BUFFER_SIZE]; 
static int mask[KFETCH_NUM_INFO] = {1, 1, 1, 1, 1, 1};

static int kfetch_open(struct inode*, struct file*);
static int kfetch_release(struct inode*, struct file*);
static ssize_t kfetch_read(struct file *, char __user *, size_t , loff_t *);
static ssize_t kfetch_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations fops = {
    .open = kfetch_open,
    .release = kfetch_release,
    .read = kfetch_read,
    .write = kfetch_write,
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("313551099");
MODULE_DESCRIPTION("A kernel module for kfetch");
MODULE_VERSION("1.0");

//---------------------------------------------------------------------------
static int __init kfetch_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "kfetch_mod: Failed to register a major number\n");
        return major_number;
    }

	// Modified on 2025/01/11------------------------------------------------
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0) 
	kfetch_class = class_create(CLASS_NAME); 
#else 
	kfetch_class = class_create(THIS_MODULE, CLASS_NAME); 
#endif 
	device_create(kfetch_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME); 

    // kfetch_class = class_create(CLASS_NAME);
    // if (IS_ERR(kfetch_class)) {
    //     unregister_chrdev(major_number, DEVICE_NAME);
    //     printk(KERN_ALERT "kfetch_mod: Failed to register device class\n");
    //     return PTR_ERR(kfetch_class);
    // }
    // kfetch_device = device_create(kfetch_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    // if (IS_ERR(kfetch_device)) {
    //     class_destroy(kfetch_class);
    //     unregister_chrdev(major_number, DEVICE_NAME);
    //     printk(KERN_ALERT "kfetch_mod: Failed to create device\n");
    //     return PTR_ERR(kfetch_device);
    // }
	// -----------------------------------------------------------------------
    printk(KERN_INFO "kfetch_mod: Module loaded.\n");
    return 0;
}

static void __exit kfetch_exit(void)
{
    device_destroy(kfetch_class, MKDEV(major_number, 0));
    class_unregister(kfetch_class);
    class_destroy(kfetch_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "kfetch_mod: Module unloaded.\n");
}

static int kfetch_open(struct inode* inodep, struct file* filep) 
{
    printk(KERN_INFO "kfetch_mod: Device opened\n");
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) 
        return -EBUSY; 
    try_module_get(THIS_MODULE);  
    return 0;
}

static int kfetch_release(struct inode* inodep, struct file* filep) 
{
    printk(KERN_INFO "kfetch_mod: Device closed\n");
    atomic_set(&already_open, CDEV_NOT_USED); 
    module_put(THIS_MODULE); 
    return 0;
}

//---------------------------------------------------------------------------
void get_kernel_release(char *info_buffer, int info_len)
{
	strncat(info_buffer, "Kernel:   ", info_len);
	strncat(info_buffer, utsname()->release, info_len);
}

void get_cpu_model(char *info_buffer, int info_len)
{
	struct cpuinfo_x86 *c = &cpu_data(0);
    strncat(info_buffer, "CPU:      ", info_len);
    strncat(info_buffer, c->x86_model_id, info_len);
}

void get_cpu_nums(char *info_buffer, int info_len)
{
	char online_cpu[30];
	char total_cpu[30];
	snprintf(online_cpu, sizeof(online_cpu), "%d",num_online_cpus());
	snprintf(total_cpu, sizeof(total_cpu), "%d", num_possible_cpus());
	strncat(info_buffer, "CPUs:     \0", info_len);
	strncat(info_buffer, online_cpu, info_len);
	strncat(info_buffer, " / ", info_len);
	strncat(info_buffer, total_cpu, info_len);
}

void get_mem_info(char *info_buffer, int info_len)
{
	// Get system info
	struct sysinfo sysInfo;
	si_meminfo(&sysInfo);
	unsigned long int total_mem = (sysInfo.totalram << (PAGE_SHIFT - 10)) / 1024;
	unsigned long int free_mem = (sysInfo.freeram << (PAGE_SHIFT - 10)) / 1024;

	// Change to string
	char total_mem_str[30];
	char free_mem_str[30];
	snprintf(total_mem_str, sizeof(total_mem_str), "%ld", total_mem);
	snprintf(free_mem_str, sizeof(free_mem_str), "%ld", free_mem);

	// Store to buffer
	strncat(info_buffer, "Mem:      \0", info_len);
	strncat(info_buffer, free_mem_str, info_len);
	strncat(info_buffer, " MB / ", info_len);
	strncat(info_buffer, total_mem_str, info_len);
	strncat(info_buffer, " MB", info_len);
}

void get_proc_nums(char *info_buffer, int info_len)
{
	// Count the processes
	int proc_count = 0;
    struct task_struct *task;
    for_each_process(task)
	{
        proc_count++;
    }

	// Change to string and store to buffer
	char procs[30];
	snprintf(procs, sizeof(procs), "%d", proc_count);
	strncat(info_buffer, "Procs:    \0", info_len);
	strncat(info_buffer, procs, info_len);
}

void get_uptime(char *info_buffer, int info_len)
{
	// Get uptime
    struct timespec64 uptime64; 
    ktime_get_boottime_ts64(&uptime64); 

	// Change to string and store to buffer
	char uptimeStr[30];
	snprintf(uptimeStr, sizeof(uptimeStr), "%lld", (long long)(uptime64.tv_sec/60));
	strncat(info_buffer, "Uptime:   ", info_len);
	strncat(info_buffer, uptimeStr, info_len);
	strncat(info_buffer, " mins\0", info_len);
}

void get_sysinfo(char *info_buffer, int infoID)
{
	size_t info_len = sizeof(info_buffer) - strlen(info_buffer) - 1;
	switch(infoID){
		case 0:
			get_kernel_release(info_buffer, info_len);
			break;
		case 1:
			get_cpu_model(info_buffer, info_len);
			break;
		case 2:
			get_cpu_nums(info_buffer, info_len);
			break;
		case 3:
			get_mem_info(info_buffer, info_len);
			break;
		case 4:
			get_proc_nums(info_buffer, info_len);
			break;
		case 5:
			get_uptime(info_buffer, info_len);
			break;
		default:
			break;
	}
}

void print_iter(int index, int *id, char *info_buffer, size_t info_len)
{
	// Print the logo auf der linken
    strncat(info_buffer, logo[index], info_len);

	// Print the info auf der rechten
	while((*id) < 6)
    {
		if(mask[(*id)] == 1)
        {
			get_sysinfo(info_buffer, (*id));
			(*id) = (*id) + 1;
			break;
		}
		(*id) = (*id) + 1;
	}
	strncat(info_buffer, "\n", info_len);
}
//---------------------------------------------------------------------------

static ssize_t kfetch_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset)
{
	// Mutex
	if (!mutex_trylock(&kfetch_mutex)) 
	{
        printk(KERN_ALERT "kfetch_mod: Device is busy\n");
        return -EBUSY;
    }

	// Clear the info_buffer
    size_t info_len = sizeof(info_buffer) - strlen(info_buffer) - 1;
	memset(info_buffer, 0, sizeof(info_buffer) - 1);

	// Print logo[7] and hostname
	strncat(info_buffer, logo[7], info_len);
	strncat(info_buffer, utsname()->nodename, info_len);
	strncat(info_buffer, "\n", info_len);

	// Print logo[0] and the dash lines
	strncat(info_buffer, logo[0], info_len);
	for(int i = 0;i < strlen(utsname() -> nodename); i++)
		strncat(info_buffer, "-", info_len);
	strncat(info_buffer, "\n", info_len);

	// Print the following logo and system info
	int id = 0;
	for(int i=1; i<=6; i++)
		print_iter(i, &id, info_buffer, info_len);
	
	// Copy to user
	ssize_t ret = copy_to_user(buffer, info_buffer, strlen(info_buffer));

	mutex_unlock(&kfetch_mutex);
    return strlen(info_buffer); 
} 

static ssize_t kfetch_write(struct file *filp, const char __user *buffer, size_t len, loff_t *offset) 
{
	// Mutex
	if (!mutex_trylock(&kfetch_mutex)) 
	{
        printk(KERN_ALERT "kfetch_mod: Device is busy\n");
        return -EBUSY;
	}

	// Clear the mask
	for(int i = 0;i < KFETCH_NUM_INFO;i++)
		mask[i] = 0;
		
	// Copy from user
	int mask_info;
	if(copy_from_user(&mask_info, buffer, len))
	{
		printk(KERN_ALERT "kfetch_mod: Failed to copy data from user space\n");
        return -EFAULT;
	}

	// Define the mask
	if(mask_info & KFETCH_RELEASE)
		mask[0] = 1;	
	if(mask_info & KFETCH_NUM_CPUS)
		mask[2] = 1;
	if(mask_info & KFETCH_CPU_MODEL)
		mask[1] = 1;
	if(mask_info & KFETCH_MEM)
		mask[3] = 1;
	if(mask_info & KFETCH_UPTIME)
		mask[5] = 1;
	if(mask_info & KFETCH_NUM_PROCS)
		mask[4] = 1;

	mutex_unlock(&kfetch_mutex);
    return 0;
}
//---------------------------------------------------------------------------

module_init(kfetch_init);
module_exit(kfetch_exit);
