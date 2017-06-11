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
static char * buffer = NULL;
static unsigned int count=0;
static dev_t dev_id;
static unsigned int led_major = 0;
static struct led_dev *led_devs = NULL;
static struct class *led_class = NULL;

static int rwbuf_open(struct inode *inode, struct file *filep)
{
  if (buffer == NULL) {
    printk("LED CONTROLLER: alloc space for buffer\n");
    buffer = kmalloc(1024,GFP_ATOMIC);
    if (buffer == NULL) {
      printk("LED CONTROLLER: Still NULL\n");
      return 1;
    }
    strcpy(buffer,"12345\n");
  }
  try_module_get(THIS_MODULE);
  ++count;
  return 0;
}

// the close
static int rwbuf_close(struct inode *inode, struct file *filep)
{
  --count;
  if(count == 0) {
    kfree(buffer);
    buffer = NULL;
  }
  module_put(THIS_MODULE);
  return 0;
}

static ssize_t rwbuf_write(struct file *filep, const char *buf, size_t count, loff_t *ppos)
{
  char *rdp = buf;
  for(size_t i = 0; i< count; ++i) {
    if(*rdp = '\0') {
      rdp = buf;
    }
    buffer[count % 1024] = *rdp;
    ++rdp;
  }
  return count;
}

static ssize_t rwbuf_read(struct file *filep, char *buf, size_t count, loff_t *ppos)
{
  char * rdp = buf;
  size_t i = 0;
  while(rdp != '\0') {
    *rdp = buffer[i];
    ++i;
    ++rdp;
  }
  return count-i;
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
  led_class = class_create(THIS_MODULE,"qled");
  if (IS_ERR(led_class)) {
    printk("LED CONTROLLER: fail to create class %d",PTR_ERR(led_class));
    return -24;
  }
  led_devs = (struct led_dev*)kzalloc(sizeof(struct led_dev),GFP_KERNEL);
  if (led_devs == NULL) {
    printk("LED CONTROLLER: fail to create devs");
    return -1;
  }
  dev_t devno = MKDEV(led_major,1);
  struct device * device = NULL;
  cdev_init(&led_devs->cdev,&rwbuf_fops);
  led_devs->cdev.owner = THIS_MODULE;
  if(cdev_add(&led_devs->cdev,devno,1)) {
    printk("LED_CONTROLLER: fail to add to devno");
    return -31;
  }
  device = device_create(led_class,NULL,devno,NULL,"qled%d",1);
  if(IS_ERR(device)) {
    printk("LED CONtroller: fail to create device %d",PTR_ERR(device));
  }
  return 0;
}

//// MODULE CLEANUP
static void exit_ledc(void) {
  printk("LED CONTROLLER: exit!\n");
  device_destroy(led_class,dev_id);
  cdev_del(&led_devs->cdev);
  class_destroy(led_class);
  unregister_chrdev_region(dev_id,1);
}

module_init(init_ledc);
module_exit(exit_ledc);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Qinka<qinka@live.com>");
