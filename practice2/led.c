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
static unsigned long buffer_size;

///// Datas
static char *buffer;
struct proc_dir_entty *qled;
static unsigned int count=0;

// static dev_t dev_id;
// static unsigned int led_major = 0;
// static struct led_dev* led_devs;

// proc open
static int qled_open(struct inode* inode, struct file *file) {
  if(count ==0 || buffer == NULL) {
    if(buffer_size == 0) {
      buffer_size = BUF_SIZE;
    }
    buffer = kmalloc(buffer_size,GFP_KERNEL);
    if(buffer == NULL) {
      printk("LED CONTROLLER: can not alloc");
      return -1;
    }
  }
  try_module_get(THIS_MODULE);
  count ++;
  return 0;
}


// proc close
static int qled_close(struct inode *inode, struct file *filep)
{
  --count;
  if(count == 0) {
    if(!buffer)
      kfree(buffer);
  }
  module_put(THIS_MODULE);
  return 0;
}

static ssize_t qled_write(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
  int len = -ENOMEM;
  if(buffer) {
    if (*f_pos < buf_size) {
        len = count + *f_pos > buf_size ? buf_size - *f_pos : count;
        copy_from_user(buffer + *f_pos,buf,len);
        printk("LED CONTROLLER: write  (A)");
    }
    else {
        len = count > buf_size ? buf_size  : count;
        copy_from_user(buffer + *f_pos,buf,len);
        printk("LED CONTROLLER: write (B)");
    }
  }
  printk("LED CONTROLLER: write %d\n", len);
  return len;
}

static ssize_t qled_read(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
  int len = 0;
  if(buffer) {
    if (*f_pos < buf_size) {
        len = count + *f_pos > buf_size ? buf_size - *f_pos : count;
        copy_to_user(buf,buffer + *f_pos,len);
        *f_pos += len;
    }
  }
  printk("LED CONTROLLER: read %d\n", len);
  return len;
}


// file operations
static struct file_operations qled_fops = {
  open:    qled_open,
  release: qled_close,
  read:    qled_read,
  write:   qled_write,
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
}

module_init(init_ledc);
module_exit(exit_ledc);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Qinka<qinka@live.com>");
