/******************************
 * 
 *  Led Controller
 *  Kernal Module
 *
 *  Copyright (C) Qinka 2017
 *  qinka@live.com me@qinka.pro
 *
 *  LED HEADER with proc
 *
 ******************************
 */

#ifndef _LED_H_
#define _LED_H_

#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include<linux/seq_file.h>
#include<linux/proc_fs.h>
#include<linux/sched.h>
#include<linux/uaccess.h>
#include<linux/string.h>
#include<linux/moduleparam.h>


#define BUF_SIZE (1024)


#endif // _LED_H_
