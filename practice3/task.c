/**********************************
 *
 * Tasklet
 * Kernel Module
 *
 * Copyright (C) 2017 Qinka
 * qinka@live.com me@qinka.pro
 *
 * TASK 
 *
 **********************************
 */


#include "task.h"


//// Datas

// display string
#define TASKLET_PUT_STR_L ("8888")
static char _tasklet_str[] = TASKLET_PUT_STR_L;
#define TASKLET_PUT_STR (_tasklet_str)
#define TASKLET_PUT_STR_SIZE (sizeof(TASKLET_PUT_STR)+1)
#define WORKQUEUE_PUT_STR_L ("1111")
static char _workqueue_str[] = WORKQUEUE_PUT_STR_L;
#define WORKQUEUE_PUT_STR (_workqueue_str)
#define WORKQUEUE_PUT_STR_SIZE (sizeof(TASKLET_PUT_STR)+1)

// tasklet
void tasklet_worker(unsigned long);
struct tasklet_struct task;

// workqueue
void workqueue_worker(void *);
struct workqueue_struct* workq;
static DECLARE_WORK(workw,workqueue_worker);

// timer
// for tasklet
void timer_tasklet(unsigned long);
struct timer_list tasklet_timer;
#define TIMER_TASKLET_DELAY (7*HZ);
// for workqueue
void timer_workqueue(unsigned long);
struct timer_list workq_timer;
#define TIMER_WORKQUEUE_DELAY (9*HZ);


//// methods
// write /dev/qled
void write_dev(void *buf, size_t size) {
  struct file* filp = filp_open("/dev/qled",O_RDWR,0666);
  if(filp) {
    mm_segment_t old_fs;
    old_fs = get_fs();
    set_fs(get_fs());
    if(!filp->f_op->write(filp,buf,size,&filp->f_pos))
      printk("TASK: fail to write\n");
    set_fs(old_fs);
    filp_close(filp,NULL);
  }
  else
    printk("TASK: fail to open /dev/qled\n");
}

// tasklet worker
void tasklet_worker(unsigned long data) {
  write_dev(&TASKLET_PUT_STR,TASKLET_PUT_STR_SIZE);
}

// workqueue
void workqueue_worker(void * data) {
  write_dev(&WORKQUEUE_PUT_STR,WORKQUEUE_PUT_STR_SIZE);
}

void timer_tasklet(unsigned long arg) {
  tasklet_schedule(&task);
  struct timer_list* pTimer = (struct timer_struct *)arg;
  pTimer->expires += TIMER_TASKLET_DELAY;
  add_timer(pTimer);
}

void timer_workqueue(unsigned long arg) {
  if(queue_work(workq,&workw) != 1)
    printk("TASK: fail to add workqueue\n");
  struct timer_list* pTimer = (struct timer_struct *)arg;
  pTimer->expires += TIMER_TASKLET_DELAY;
  add_timer(pTimer);
}

// INIT
static int init_task_m(void) {
  printk("TASK: start\n");
  init_timer(&tasklet_timer);
  init_timer(&workq_timer);
  unsigned long j = jiffies;
  tasklet_timer.expires  = j + TIMER_TASKLET_DELAY;
  tasklet_timer.function = timer_tasklet;
  tasklet_timer.data     = (unsigned long)(&tasklet_timer);
  workq_timer.expires    = j + TIMER_WORKQUEUE_DELAY;
  workq_timer.function   = timer_workqueue;
  workq_timer.data       = (unsigned long)(&workq_timer);
  tasklet_init(&task,tasklet_worker,NULL);
  workq = create_singlethread_workqueue("led_task");
  add_timer(&tasklet_timer);
  add_timer(&workq_timer);
}


// EXIT
static void exit_task_m(void) {
  printk("TASK: end\n");
  del_timer(&tasklet_timer);
  del_timer(&workq_timer);
  tasklet_kill(&task);
  destroy_workqueue(workq);
}

module_init(init_task_m);
module_exit(exit_task_m);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Qinka<qinka@live.com");
