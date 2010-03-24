/*
 *  fpga_drv.c
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/ioport.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#include <linux/wrapper.h>
#endif

#define INTERRUPT 9

#define fpga_VERSION "1.0"
#define fpga_MAJOR   246
#define fpga_NAME    "fpga_drv"

#define FPGA_BASE 0xD3000000
#define FPGA_MASK 0x00ffffff
#define FPGA_SIZE 0x01000000

#define COMMAND_MASK 0x80000000
#undef DEBUG

volatile unsigned int *fpga_ptr;
unsigned int offset;
int interruptcount = 0;
static struct proc_dir_entry *interrupt_arm_file;
static struct fasync_struct *fasync_fpga_queue ;

DECLARE_WAIT_QUEUE_HEAD(fpga_wait);

irqreturn_t interrupt_interrupt_arm(int irq, void *dev_id, struct pt_regs *regs)
{

   interruptcount++;
   printk(KERN_INFO "\nfpga_drv: Interrupt detected in kernel \n");

   /* acknowledge the interrupt by reading the result value */
   readl(fpga_ptr);

   /* Signal the user application that an interupt occured */
   kill_fasync(&fasync_fpga_queue, SIGIO, POLL_IN);

   return IRQ_HANDLED;
}

static int proc_read_interrupt_arm(char *page, char **start, off_t off, int count, int *eof, void *data)
{
  int len;

  len = sprintf(page, "Total number of interrupts %19i\n", interruptcount);

  return len;
}

static int fpga_open1 (struct inode *inode, struct file *file) {
   return 0;
}

static int fpga_release1 (struct inode *inode, struct file *file) {
   return 0;
}

static int fpga_fasync1 (int fd, struct file *filp, int on)
{

   printk(KERN_INFO "\nfpga_drv: Inside fpga_fasync \n");
   return fasync_helper(fd, filp, on, &fasync_fpga_queue);

} 

static int fpga_ioctl1(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) {

   int retval = 0;
   unsigned int value;
   unsigned int command_type;
   unsigned int offset;
   volatile unsigned int *access_addr;

   command_type = 0x80000000ul;

   // Set the offset for register accesses
   command_type = COMMAND_MASK & cmd;

   printk(KERN_INFO "\nfpga_drv: Inside fpga_ioctl1 \n");

   offset = ~COMMAND_MASK & cmd & FPGA_MASK;
   if(offset > FPGA_SIZE)
      retval=-EINVAL;

   switch(command_type)
   {
      case 0:
         //read
         if(!access_ok(VERIFY_READ, (unsigned int *)arg, sizeof(int)))
            return -EFAULT;

         *(unsigned int *)arg = readl((volatile unsigned int *)&fpga_ptr[offset]);
         printk("fpga_drv: Read value %08x\n", *(unsigned int *)arg);
         break;

      case COMMAND_MASK:
         //write
         access_addr = fpga_ptr + offset;

         if(!access_ok(VERIFY_WRITE, (unsigned int *)arg, sizeof(int)))
            return -EFAULT;

         value = *(unsigned int *)arg;
         printk("fpga_drv: Write value %08x\n", (unsigned int)value);
         writel(value, access_addr); 
         break;

      default:
         printk(KERN_ERR "fpga_drv: Invalid command \n");
         retval = -EINVAL;
   }

   return retval;
}

// define which file operations are supported
struct file_operations fpga_fops = {
   .owner=   THIS_MODULE,
   .llseek  = NULL,
   .read    = NULL,
   .write   = NULL,
   .readdir = NULL,
   .poll    = NULL,
   .ioctl   = fpga_ioctl1,
   .mmap    = NULL,
   .open    = fpga_open1,
   .flush   = NULL,
   .release = fpga_release1,
   .fsync   = NULL,
   .fasync  = fpga_fasync1,
   .lock    = NULL,
   .readv   = NULL,
   .writev  = NULL,
};

// initialize module
static int __init fpga_init_module (void) {

   int rv = 0;
   unsigned long arg = 0;
   volatile unsigned int *access_addr;

   printk("FPGA Interface Module\n");
   printk(KERN_INFO "\nfpga_drv: FPGA Driver Loading.\n");
   printk(KERN_INFO "fpga_drv: Using Major Number %d on %s\n", fpga_MAJOR, fpga_NAME); 

   if (register_chrdev(fpga_MAJOR, fpga_NAME, &fpga_fops)) 
   {
      printk("fpga_drv: unable to get major %d. ABORTING!\n", fpga_MAJOR);
      return -EBUSY;
   }

   // Perform Memory REMAP
   if (check_mem_region(FPGA_BASE, FPGA_SIZE))
   {
      printk(KERN_ERR "fpga_drv: Unable to acquire FPGA address.\n");
      return -EBUSY;
   }

   request_mem_region(FPGA_BASE, FPGA_SIZE, fpga_NAME);
   fpga_ptr = (volatile unsigned int *)__ioremap(FPGA_BASE, FPGA_SIZE, 0);

   if (!fpga_ptr)
   {
      printk(KERN_ERR "fpga_drv: Unable to map FPGA.\n");
      release_mem_region(FPGA_BASE, FPGA_SIZE);

      return -EBUSY;
   }

   printk("fpga_drv: %08x size %08x mapped to %08x\n", FPGA_BASE, FPGA_SIZE, (unsigned int)fpga_ptr);

   interrupt_arm_file = create_proc_entry("interrupt_arm", 0444, NULL);

   if(interrupt_arm_file == NULL)
   {
      printk("fpga_drv: create /proc entry returned NULL. ABORTING!\n");
      return -ENOMEM;
   }

   interrupt_arm_file->data = NULL;
   interrupt_arm_file->read_proc = &proc_read_interrupt_arm;
   interrupt_arm_file->write_proc = NULL;
   interrupt_arm_file->owner = THIS_MODULE;

   /* request interrupt from linux */
   rv = request_irq(INTERRUPT, interrupt_interrupt_arm, SA_TRIGGER_RISING, "interrupt_arm", NULL);
  
   if ( rv )
   {
      printk("fpga_drv: Can't get interrupt %d: %d\n", INTERRUPT, rv);
      goto no_interrupt_arm;
   }

   /* everything initialized */
   printk(KERN_INFO "fpga_drv: %s %s Initialized\n", fpga_NAME, fpga_VERSION);
   return 0;

   /* remove the proc entry on error */
   no_interrupt_arm:
   remove_proc_entry("interrupt_arm", NULL);
   return -EBUSY;

}

static void __exit fpga_cleanup_module (void) {

   free_irq(INTERRUPT,NULL);

   iounmap((void *)fpga_ptr);
   release_mem_region(FPGA_BASE, FPGA_SIZE);
   printk("fpga_drv: Device released.\n");

   unregister_chrdev (fpga_MAJOR, fpga_NAME);

   remove_proc_entry("interrupt_arm", NULL);
   printk(KERN_INFO "fpga_drv: %s %s removed\n", fpga_NAME, fpga_VERSION);

}

module_init(fpga_init_module);
module_exit(fpga_cleanup_module);
MODULE_AUTHOR("gerstl@ece.utexas.edu");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("FPGA Device Driver");
