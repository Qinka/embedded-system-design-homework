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

static char * pages;

static int rwbuf_open(struct inode *inode, struct file *filep) {
  printk("LED CONTROLLER: open %u\n",count);
  if(count == 0 || pages == NULL) {
    pages = __get_free_pages(GFP_KERNEL,PAGE_COUNT);
    if(pages == NULL){
      printk("LED CONTROLLER: can not alloc\n");
      return -1;
    }
    printk("LED CONTROLLER: alloc\n");
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
    if(pages) {
      free_pages(pages,PAGE_COUNT);
      printk("LED CONTROLLER: free\n");
      pages = NULL;
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
      newpos =  PAGE_T_SIZE + o;
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
  if(pages) {
    if (*f_pos < PAGE_T_SIZE) {
        len = count + *f_pos > PAGE_T_SIZE ? PAGE_T_SIZE - *f_pos : count;
        copy_from_user(pages + *f_pos,buf,len);
    }
    else {
        len = count > PAGE_T_SIZE ? PAGE_T_SIZE : count;
        copy_from_user(pages + *f_pos,buf,len);
    }
    *f_pos += len;
  }
  return len;
}

static ssize_t rwbuf_read(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
  int len = 0;
  if(pages) {
    if (*f_pos < PAGE_T_SIZE) {
        len = count + *f_pos > PAGE_T_SIZE ? PAGE_T_SIZE - *f_pos : count;
        copy_to_user(buf,pages + *f_pos,len);
    }
    *f_pos += len;
  }
  return len;
}

// vma
static void rwbuf_vma_open(struct vm_area_struct *vma) {
  printk("LED CONTROLLER: led VMA open, virt %lx, phys %lx\n",vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
}

static void rwbuf_vma_close(struct vm_area_struct *vma) {
  printk("LED CONTROLLER: led VMA close\n");
}

static struct page* rwbuf_vma_fault(struct vm_area_struct *vma, struct vm_fault *vmf) {
  struct page *page;
  if (vmf->pgoff >= (1 << PAGE_COUNT))
    return VM_FAULT_SIGBUS;
  unsigned long viraddr = pages + (unsigned long)(vmf->virtual_address) - vma->vm_start  + (vma->vm_pgoff << PAGE_SHIFT);
  page = virt_to_page(viraddr);
  get_page(page);
  if (vma->vm_file)
    page->mapping = vma->vm_file->f_mapping;
  else
    printk(KERN_ERR "no mapping available\n");
  vmf->page = page;
  return 0;
}

static struct vm_operations_struct rwbuf_remap_vm_ops = {
 open  : rwbuf_vma_open,
 close : rwbuf_vma_close,
 fault : rwbuf_vma_fault,
};
  
static int rwbuf_fault_mmap(struct file *filp, struct vm_area_struct *vma) {
  unsigned long offset = vma -> vm_pgoff << PAGE_SHIFT;
  if(offset >= __pa(high_memory) || (filp->f_flags & O_SYNC))
    vma->vm_flags |= VM_IO;
  vma->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP);

  vma->vm_ops = &rwbuf_remap_vm_ops;
  rwbuf_vma_open(vma);
  return 0;
}



// file operations
static struct file_operations rwbuf_fops = {
 open:    rwbuf_open,
 release: rwbuf_close,
 read:    rwbuf_read,
 write:   rwbuf_write,
 llseek:  rwbuf_llseek,
 mmap:    rwbuf_fault_mmap,
};



//// MODULE INIT
static int init_ledc(void) {
  pages = NULL;
  count = 0;
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
  if(pages) {
    free_pages(pages,PAGE_COUNT);
    printk("LED CONTROLLER: free");
  }
}

module_init(init_ledc);
module_exit(exit_ledc);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Qinka<qinka@live.com>");
