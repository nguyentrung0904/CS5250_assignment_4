#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#define MAJOR_NUMBER 61
#define SCULL_IOC_MAGIC  'k' 
#define SCULL_HELLO _IO(SCULL_IOC_MAGIC,   1) 
#define MAJOR_NUM 100

#define IOCTL_SET_MSG _IOR(MAJOR_NUM, 0, char *)

#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, char *)
#define BUF_LEN 1024*4*1024
int onebyte_open(struct inode *inode, struct file *filep);
int onebyte_release(struct inode *inode, struct file *filep);
ssize_t onebyte_read(struct file *filep, char *buf, size_t count, loff_t *f_pos);
ssize_t onebyte_write(struct file *filep, const char *buf,
size_t count, loff_t *f_pos);
loff_t onebyte_lseek(struct file *filp, loff_t offset, int whence);
long onebyte_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) ;
static void onebyte_exit(void);

struct file_operations onebyte_fops = {
	read: onebyte_read,
	write: onebyte_write,
	open: onebyte_open,
	unlocked_ioctl: onebyte_ioctl,
	release: onebyte_release,
	llseek: onebyte_lseek


};

char* onebyte_data = NULL;
char* msg_Ptr = NULL;
int current_size = 0;
int onebyte_open(struct inode *inode, struct file *filep)
{
	printk(KERN_NOTICE "Open device\n");
	msg_Ptr = onebyte_data;
	return 0; // always successful
}

int onebyte_release(struct inode *inode, struct file *filep)
{
	return 0; // always successful
}


int times = 0;


ssize_t onebyte_read(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
	int bytes_read = 0;

  	if (*msg_Ptr == 0)
	    return 0;
	
	printk("offset: %d\n", *f_pos);
  	while (count && *msg_Ptr)  {

	    	put_user(*(msg_Ptr++), buf++);
	    	count--;
	    	bytes_read++;

	}
	  	return bytes_read;
}
/*

	printk(KERN_NOTICE "data: %s\n", msg_Ptr);
	printk("count: %d\n", count);
	if (*msg_Ptr == 0)
		return 0;
	put_user(*(msg_Ptr), buf);
	msg_Ptr++;
	printk(KERN_NOTICE "Read from device %d\n", times);

	times++;

	return 1;
*/


ssize_t onebyte_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos)
{	
	printk("Write to device func called\n");
	printk("count: %d\n", count);
	printk("buffer: %s\n", buf);
/*
	*msg_Ptr = buf[0];

	if (count == 1){
		*msg_Ptr = '\0';
	}
	printk("data: %s\n", onebyte_data);
	msg_Ptr++;
	if (count>1024*4-1){
		printk(KERN_NOTICE "No more memory\n");
		return -28;
	} 
	
    	return 1;
*/
	int i;
	for(i=0; i<BUF_LEN && i<count; i++)
    		get_user(msg_Ptr[i], buf+i);
	onebyte_data = msg_Ptr;

	return i;
}


loff_t onebyte_lseek(struct file *file, loff_t offset, int whence)
{

    printk("Llseek to device func called\n");
        loff_t new_pos=0;
	int bytes_read = 0;
	msg_Ptr = onebyte_data;
	while (*msg_Ptr)  {
		msg_Ptr++;
	    	bytes_read++;
		//printk("%c ", msg_Ptr[0]);
	}

	printk("size: %d\n", bytes_read);
            switch( whence )

            {

            case 0: //SET

                        new_pos = offset;

                        break;

            case 1: //CURRENT

                        new_pos = file->f_pos + offset;

                        break;

            case 2: //END

                        new_pos = bytes_read - offset;
                        break;
            }          
            if( new_pos > bytes_read ) new_pos = bytes_read;
            if( new_pos < 0 ) new_pos = 0;
            file->f_pos = new_pos;
	printk("new position %d ", new_pos);
            return new_pos;
}


long onebyte_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) 
{ 
 printk(KERN_WARNING "ioctl called\n");
   int err = 0, tmp; 
    int retval = 0; 
	printk(KERN_WARNING "cmd = %d ", cmd);

int i;
	char *temp;
	char ch;

    switch(cmd) { 
            
	case 2:

		temp = (char *)arg;

		get_user(ch, temp);
		for (i = 0; ch && i < BUF_LEN; i++, temp++)
			get_user(ch, temp);

		onebyte_write(filp, (char *)arg, i, 0);
		break;

	case 3:

		i = onebyte_read(filp, (char *)arg, 99, 0);

		/* 
		 * Put a zero at the end of the buffer, so it will be 
		 * properly terminated 
		 */
		put_user('\0', (char *)arg + i);
		break;
	case 1: 
        printk(KERN_WARNING "hello\n"); 
        break;  
      default:  /* redundant, as cmd was checked against MAXNR */ 
    
 return -ENOTTY; 
    } 
    return retval; 
 
} 





static int onebyte_init(void)
{
	int result;
	// register the device
	result = register_chrdev(MAJOR_NUMBER, "onebyte",
	&onebyte_fops);
	if (result < 0) {
		return result;
	}
	onebyte_data = kmalloc(sizeof(char)*BUF_LEN, GFP_KERNEL);
	if (!onebyte_data) {
		onebyte_exit();
		return -ENOMEM;
	}
	sprintf(onebyte_data, "");
	printk(KERN_ALERT "This is a 4MB device module\n");
	return 0;
}



static void onebyte_exit(void)
{
	if (onebyte_data) {
		kfree(onebyte_data);
		onebyte_data = NULL;
	}
	// unregister the device
	unregister_chrdev(MAJOR_NUMBER, "onebyte");
	printk(KERN_ALERT "4MB device module is unloaded\n");
}



MODULE_LICENSE("GPL");
module_init(onebyte_init);
module_exit(onebyte_exit);

