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


static int rwbuf_open(struct inode *inode, struct file *filep)
{
  if (buffer == NULL) {
    printk("LED CONTROLLER: alloc space for buffer\n");
    buffer = vmalloc(1024);
    if (buffer == NULL) {
      printk("LED CONTROLLER: Still NULL\n");
      return 1;
    }
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
    vfree(buffer);
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
static int init_module() {
  printk("LED CONTROLLER: start!\n");
  alloc_chrdev_region(&dev_id,0,1,"/dev/qled");
  return 0;
}

//// MODULE CLEANUP
static void exit_module() {
  printk("LED CONTROLLER: exit!\n");
  unregister_chrdev_region(dev_id,1);
}

module_init(init_module);
module_exit(exit_module);

MODULE_LICENSE("GPL3");
MODULE_AUTHOR("Qinka<qinka@live.com>");
