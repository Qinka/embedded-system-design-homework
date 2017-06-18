/******************************
 * 
 *  Led Controller
 *  Kernal Module
 *
 *  Copyright (C) Qinka 2017
 *  qinka@live.com me@qinka.pro
 *
 *  LED CONTROLLER with proc
 *
 ******************************
 */

#include "led.h"

// params
static unsigned long buffer_size = 0;

///// Datas
static char *buffer;
struct proc_dir_entty *qled;
static unsigned int count=0;

// static dev_t dev_id;
// static unsigned int led_major = 0;
// static struct led_dev* led_devs;

// proc open
static int qled_open(struct inode* inode, struct file *file) {
  printk("LED CONTROLLER: open %u\n",count);
  if(count ==0 || buffer == NULL) {
    if(buffer_size == 0) {
      buffer_size = BUF_SIZE;
    }
    buffer = kmalloc(buffer_size,GFP_KERNEL);
    if(buffer == NULL) {
      printk("LED CONTROLLER: can not alloc");
      return -1;
    }
    printk("LED CONTROLLER: alloc\n");
    buffer_size=BUFFER_SIZE;
  }
  try_module_get(THIS_MODULE);
  ++ count;
  return 0;
}


// proc close
static int qled_close(struct inode *inode, struct file *filep)
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

static loff_t qled_llseek(struct file *file,loff_t o,int whence) {
  loff_t newpos;
  switch (whence) {
    case SEEK_SET:
      newpos = o;
      break;
    case SEEK_CUR:
      newpos = file->f_pos + o;
      break;
    case SEEK_END:
      newpos = buffer_size + o;
      break;
    default:
      return -EINVAL;
  }
  if(newpos <0) return -EINVAL;
  file->f_pos = newpos;
  return newpos;
}

static ssize_t qled_write(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
  int len = -ENOMEM;
  if(buffer) {
    if (*f_pos < buffer_size) {
        len = count + *f_pos > buffer_size ? buffer_size - *f_pos : count;
        copy_from_user(buffer + *f_pos,buf,len);
    }
    else {
        len = count > buffer_size ? buffer_size  : count;
        copy_from_user(buffer + *f_pos,buf,len);
    }
    *f_pos += len;
  }
  return len;
}

static ssize_t qled_read(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
  int len = 0;
  if(buffer) {
    if (*f_pos < buffer_size) {
        len = count + *f_pos > buffer_size ? buffer_size - *f_pos : count;
        copy_to_user(buf,buffer + *f_pos,len);
    }
    *f_pos += len;
  }
  return len;
}


// file operations
static struct file_operations qled_fops = {
  open:    qled_open,
  release: qled_close,
  read:    qled_read,
  write:   qled_write,
  llseek:  qled_llseek,
  };



//// MODULE INIT
static int init_ledc(void) {
  printk("LED CONTROLLER: start!\n");
  qled = proc_create_data("qled",0644,NULL,&qled_fops,"123");
  return 0;
}

//// MODULE CLEANUP
static void exit_ledc(void) {
  printk("LED CONTROLLER: exit!\n");
  remove_proc_entry("qled",NULL);
  if(buffer) {
    kfree(buffer);
    printk("LED CONTROLLER: free");
  }
}

module_init(init_ledc);
module_exit(exit_ledc);

module_param(buffer_size,ulong,S_IRUGO);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Qinka<qinka@live.com>");
