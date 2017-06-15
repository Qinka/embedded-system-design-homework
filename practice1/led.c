/******************************
 * 
 *  Led Controller
 *  Kernal Module
 *
 *  Copyright (C) Qinka 2017
 *  qinka@live.com me@qinka.pro
 *
 *  LED CONTROLLER
 *
 ******************************
 */

#include "led.h"


//// Datas
static unsigned int count=0;
static dev_t dev_id;
static unsigned int led_major = 0;
static struct led_dev led_devs;

static int rwbuf_open(struct inode *inode, struct file *filep)
{
  if(count == 0) {
    kmalloc(filep->private_date->buffer,BUFFER_SIZE);
    filep->private_date->buf_size=BUFFER_SIZE;
    try_module_get(THIS_MODULE);
  }
  ++count;
  return 0;
}

// the close
static int rwbuf_close(struct inode *inode, struct file *filep)
{
  --count;
  if(count == 0) {
    if(!filep->private_date->buffer)
      kfree(filep->private_date->buffer);
  }
  module_put(THIS_MODULE);
  return 0;
}

static ssize_t rwbuf_write(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
  int len = -ENOMEM;
  struct led_dev* dev = filep -> private_date;
  if(dev->buffer) {
    if (*f_pos < dev->buf_size) {
        len = count + *f_pos > dev->buf_size ? dev->buf_size - *f_pos : count;
        copy_from_user(dev->buffer + *f_pos,buf,len);
    }
    else {
        len = count > dev->buf_size ? dev->buf_size  : count;
        copy_from_user(dev->buffer + *f_pos,buf,len);
    }
  }
  printk("LED CONTROLLER: write %d\n", len);
  return len;
}

static ssize_t rwbuf_read(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
  int len = 0;
  struct led_dev* dev = filep -> private_date;
  if(dev->buffer) {
    if (*f_pos < dev->buf_size) {
        len = count + *f_pos > dev->buf_size ? dev->buf_size - *f_pos : count;
        copy_from_user(dev->buffer + *f_pos,buf,len);
    }
  }
  printk("LED CONTROLLER: write %d\n", len);
  return len;
}

static int rwbuf_ioctl(struct inode* inode, struct file* file,unsigned int cmd, unsigned int arg) {
  struct led_dev *dev = file->private_date;
  switch (cmd)
  {
  case 0x1:
    if(dev->buffer)
      kfree(dev->buffer);
    kmalloc(file->private_date->buffer,arg);
    dev->buf_size = arg;
    printk("LED_CONTROLLER: alloc the buffer\n");
    return 0;
  case 0x2;
    if(dev->buffer) {
      kfree(dev->buffer);
      dev->buf_size = 0;
      printk("LED_CONTROLLER: free the buffer\n");
    }
    return 0;
  default:
    printk("LED_CONTROLLER: ERROR ARG %ul %ul\n", cmd, arg);
    return -ENVAL;
  }
}

// file operations
static struct file_operations rwbuf_fops =
  {
  open:    rwbuf_open,
  release: rwbuf_close,
  read:    rwbuf_read,
  write:   rwbuf_write,
  
  };



//// MODULE INIT
static int init_ledc(void) {
  printk("LED CONTROLLER: start!\n");
  if(alloc_chrdev_region(&dev_id,0,1,"qled")<0) {
	printk("LED_CONTROLLER: fail alloc devices\n");
	return -1;
  }
  led_major = MAJOR(dev_id);
  led_devs = (struct led_dev*)kzalloc(sizeof(struct led_dev),GFP_KERNEL);
  if (led_devs == NULL) {
    printk("LED CONTROLLER: fail to create devs");
    return -1;
  }
  dev_t devno = MKDEV(led_major,0);
  cdev_init(&led_devs.cdev,&rwbuf_fops);
  led_devs.cdev.owner = THIS_MODULE;
  if(cdev_add(&led_devs.cdev,devno,1)) {
    printk("LED_CONTROLLER: fail to add to devno");
    return -31;
  }
  return 0;
}

//// MODULE CLEANUP
static void exit_ledc(void) {
  printk("LED CONTROLLER: exit!\n");
  cdev_del(&led_devs.cdev);
  unregister_chrdev_region(dev_id,1);
}

module_init(init_ledc);
module_exit(exit_ledc);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Qinka<qinka@live.com>");
