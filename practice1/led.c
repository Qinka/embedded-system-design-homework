/******************************
 * 
 *  Led Controller
 *  Kernel Module
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
static struct led_dev* led_devs;

static char * buffer;
static unsigned long buf_size;

static int rwbuf_open(struct inode *inode, struct file *filep) {
  printk("LED CONTROLLER: open %u\n",count);
  if(count == 0 || buffer == NULL) {
    buffer = kmalloc(BUFFER_SIZE,GFP_KERNEL);
    if(buffer == NULL){
      printk("LED CONTROLLER: can not alloc\n");
      return -1;
    }
    printk("LED CONTROLLER: alloc\n");
    buf_size=BUFFER_SIZE;
  }
  try_module_get(THIS_MODULE);
  ++ count;
  return 0;
}

// the close
static int rwbuf_close(struct inode *inode, struct file *filep)
{
  --count;
  printk("LED CONTROLLER: close %u",count);
  if(count == 0) {
    if(buffer) {
      kfree(buffer);
      printk("LED CONTROLLER: free\n");
      buffer = NULL;
    }
  }
  module_put(THIS_MODULE);
  return 0;
}

static loff_t rwbuf_llseek(struct file *file,loff_t o,int whence) {
  loff_t newpos;
  switch (whence) {
    case SEEK_SET:
      newpos = o;
      break;
    case SEEK_CUR:
      newpos = file->f_pos + o;
      break;
    case SEEK_END:
      newpos = buf_size + o;
      break;
    default:
      return -EINVAL;
  }
  if(newpos <0) return -EINVAL;
  file->f_pos = newpos;
  return newpos;
}

static ssize_t rwbuf_write(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
  int len = -ENOMEM;
  if(buffer) {
    if (*f_pos < buf_size) {
        len = count + *f_pos > buf_size ? buf_size - *f_pos : count;
        copy_from_user(buffer + *f_pos,buf,len);
    }
    else {
        len = count > buf_size ? buf_size  : count;
        copy_from_user(buffer + *f_pos,buf,len);
    }
    *f_pos += len;
  }
  return len;
}

static ssize_t rwbuf_read(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
  int len = 0;
  if(buffer) {
    if (*f_pos < buf_size) {
        len = count + *f_pos > buf_size ? buf_size - *f_pos : count;
        copy_to_user(buf,buffer + *f_pos,len);
    }
    *f_pos += len;
  }
  return len;
}

static int rwbuf_ioctl(struct inode* inode, struct file* file,unsigned int cmd, unsigned int arg) {
  switch (cmd)
  {
  case 0x1:
    if(buffer) {
      kfree(buffer);
      printk("LED CONTROLLER: free");
    }
    buffer = kmalloc(arg,GFP_KERNEL);
    buf_size = arg;
    printk("LED_CONTROLLER: alloc the buffer\n");
    return 0;
  case 0x2:
    if(buffer) {
      kfree(buffer);
      printk("LED CONTROLLER: free");
      buf_size = 0;
      printk("LED_CONTROLLER: free the buffer\n");
    }
    return 0;
  default:
    printk("LED_CONTROLLER: ERROR ARG %ul %ul\n", cmd, arg);
    return -1;
  }
}

// file operations
static struct file_operations rwbuf_fops = {
  open:    rwbuf_open,
  release: rwbuf_close,
  read:    rwbuf_read,
  write:   rwbuf_write,
  llseek:  rwbuf_llseek,
  unlocked_ioctl:   rwbuf_ioctl,
  };



//// MODULE INIT
static int init_ledc(void) {
  buffer = NULL;
  buf_size = 0;
  count = 0;
  printk("LED CONTROLLER: start!\n");
  if(alloc_chrdev_region(&dev_id,0,1,"qled")<0) {
	printk("LED_CONTROLLER: fail alloc devices\n");
	return -1;
  }
  led_major = MAJOR(dev_id);
  led_devs = (struct led_dev*)kmalloc(sizeof(struct led_dev),GFP_KERNEL);
  if (led_devs == NULL) {
    printk("LED CONTROLLER: fail to create devs");
    return -1;
  }
  dev_t devno = MKDEV(led_major,0);
  cdev_init(&led_devs->cdev,&rwbuf_fops);
  led_devs->cdev.owner = THIS_MODULE;
  if(cdev_add(&led_devs->cdev,devno,1)) {
    printk("LED_CONTROLLER: fail to add to devno");
    return -31;
  }
  return 0;
}

//// MODULE CLEANUP
static void exit_ledc(void) {
  printk("LED CONTROLLER: exit!\n");
  cdev_del(&led_devs->cdev);
  unregister_chrdev_region(dev_id,1);
  kfree(led_devs);
  if(buffer) {
    kfree(buffer);
    printk("LED CONTROLLER: free");
  }
}

module_init(init_ledc);
module_exit(exit_ledc);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Qinka<qinka@live.com>");
