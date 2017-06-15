/******************************
 * 
 *  Led Controller
 *  Kernal Module
 *
 *  Copyright (C) Qinka 2017
 *  qinka@live.com me@qinka.pro
 *
 *  LED HEADER
 *
 ******************************
 */

#ifndef _LED_H_
#define _LED_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/error.h>

#define BUFFER_SIZE (1024)

struct led_dev {
  struct cdev cdev;
  char* buffer = NULL;
  unsigned long buf_size=0;
};


#endif // _LED_H_
