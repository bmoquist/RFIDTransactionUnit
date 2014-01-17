/* RFID Kernel Module for Buttons
 * Authors: Ian Bablewski, Calvin Flegal, Bryant Moquist
 *
 */
/* Necessary includes for device drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/jiffies.h> /* jiffies */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/interrupt.h>
#include <asm/arch/gpio.h>
#include <asm/hardware.h>
#include <asm/arch/pxa-regs.h>
#include <linux/sched.h>
#include <asm/irq.h>

#define kRFID_MAJOR 61
/** Define module metadata */
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Ian Bablewski\nCalvin Flegal\nBryant Moquist");
MODULE_DESCRIPTION("Interrupts and buttons for RFID module");

/** Define kRFID file ops */
/* Define mytimer file operator function headers (open, release, read, write) */
static int kRFID_open(struct inode *inode, struct file *filp);
static int kRFID_release(struct inode *inode, struct file *filp);
static ssize_t kRFID_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t kRFID_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

/* kRFID file access structure */
struct file_operations kRFID_fops = {
    read: kRFID_read,
    write: kRFID_write,
    open: kRFID_open,
    release: kRFID_release
};

//Button interrupts
#define BT0 101
#define BT1 117
#define BT2 17
static const int gpio_BT0_irq = gpio_to_irq(BT0);
static const int gpio_BT1_irq = gpio_to_irq(BT1);
static const int gpio_BT2_irq = gpio_to_irq(BT2);

/* Prototypes */
static int kRFID_init(void);
static void kRFID_exit(void);

/* Declaration of the init and exit functions */
module_init(kRFID_init);
module_exit(kRFID_exit);

static int mode=0;
static int chargeSetting=0;
static int newValueSetting =0;

//Button 0 -- Determine the mode
// 0.  Read mode
// 1.  Charge mode
// 2.  Load new value

static irqreturn_t gpio_bt0_handler(int irq, void *dev_id) {
	if(mode < 2)
		mode++;
	else
		mode=0;
		
    return IRQ_HANDLED;
}

//Button 1 -- Charge amount
//10 different charge settings that are determined
//in the user program based on the setting

static irqreturn_t gpio_bt1_handler(int irq, void *dev_id) {
	if(chargeSetting < 9)
	{
		chargeSetting++;
	}
	else {
		chargeSetting=0;
	}
	
    return IRQ_HANDLED; // Tell the caller that the IRQ was handled correctly
}

//Button 2 -- New value
//10 different new value settings that are determined
//in the user program based on the setting

static irqreturn_t gpio_bt2_handler(int irq, void *dev_id) {
	if(newValueSetting < 9)
	{
		newValueSetting++;
	}
	else {
		newValueSetting=0;
	}
    return IRQ_HANDLED; // Tell the caller that the IRQ was handled correctly
}

static int kRFID_init(void) {
    int ret;
	int result;
    result = register_chrdev(kRFID_MAJOR, "kRFID", &kRFID_fops);

	//Set direction input for buttons
	gpio_direction_input(BT0);
	gpio_direction_input(BT1); 
	gpio_direction_input(BT2); 

    // Insert BT0 Input Interrupt (trigger on rising edge and check)
    ret = request_irq(gpio_BT0_irq, gpio_bt0_handler, 
                      IRQF_TRIGGER_RISING, 
                      "BT0 Edge Triggered Handler",gpio_bt0_handler);
    if (ret) {
        printk("BT0 interrupt request failed\n");
        goto fail;
    } else {
        printk("BT0 interrupt request succeeded\n");
    }

    // Insert BT1 Input Interrupt (trigger on rising edge and check)
    ret = request_irq(gpio_BT1_irq, gpio_bt1_handler, 
                      IRQF_TRIGGER_RISING, 
                      "BT1 Edge Triggered Handler",gpio_bt1_handler);
    if (ret) {
        printk("BT1 interrupt request failed\n");
        goto fail;
    } else {
        printk("BT1 interrupt request succeeded\n");
    }

    // Insert BT2 Input Interrupt (trigger on rising edge and check)
    ret = request_irq(gpio_BT2_irq, gpio_bt2_handler, 
                      IRQF_TRIGGER_RISING, 
                      "BT2 Edge Triggered Handler", gpio_bt2_handler);
    if (ret) {
        printk("BT2 interrupt request failed\n");
        goto fail;
    } else {
        printk("BT2 interrupt request succeeded\n");
    }
	
	return 0;

fail: 
	kRFID_exit(); 
	return 0;
}


static void kRFID_exit(void) {
	/* Free memory */	
    unregister_chrdev(kRFID_MAJOR, "kRFID");
    free_irq(gpio_BT0_irq, gpio_bt0_handler);
    free_irq(gpio_BT1_irq, gpio_bt1_handler);
    free_irq(gpio_BT2_irq, gpio_bt2_handler);
	printk(KERN_ALERT "Removing kRFID module\n");
}



// File Ops
/** Implement file operators **/
static int kRFID_open(struct inode *inode, struct file *filp)
{
    /* Success */
    return 0;
}

static int kRFID_release(struct inode *inode, struct file *filp)
{
    /* Success */
    return 0;
}


static ssize_t kRFID_write(struct file *filp, const char *buf, size_t count,
                            loff_t *f_pos)
{
    return count;
}


// Present status information to userland
static ssize_t kRFID_read( struct file *filp, char *buf, size_t count, 
                            loff_t *f_pos )
{
	int len;
    char *sbuf;
	sbuf = (char *)kmalloc(128, GFP_KERNEL); 
	
	if(*f_pos == 0)
	{
	// Need to return: mode, charge amount, and new value.
		*f_pos+= 1;
		sprintf(sbuf, "%d %d %d ", mode, chargeSetting, newValueSetting);
		
		if (copy_to_user(buf,sbuf, strlen(sbuf))) {
			kfree(sbuf);
			return -ENOMEM;
			}
		
			len = strlen(sbuf);
			kfree(sbuf);
			return len;
	}
		
	else {
		kfree(sbuf);
		return 0;
		}
}

