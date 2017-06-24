/******************************
 * 
 *  Led Controller
 *  Daemon Process
 *
 *  Copyright (C) Qinka 2017
 *  qinka@live.com me@qinka.pro
 *
 *  Daemon main process
 *
 ******************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <wiringPi.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/mman.h>


static int fd;
int daemon_process();




int pin_setup(int* pins) {
  wiringPiSetup();
  for(int i = 0; i< 8; ++i) {
    pinMode(pins[i],OUTPUT);
    digitalWrite(pins[i],i<4?LOW:HIGH);
  }
  return 0;
}



int qled_display_positive(int *pins,char c) {
#define DISPLAY(c,v,i) {if((c) & (v)) digitalWrite(pins[(i)],HIGH);else digitalWrite(pins[(i)],LOW);}
  DISPLAY(c,0x8,0);
  DISPLAY(c,0x4,1);
  DISPLAY(c,0x2,2);
  DISPLAY(c,0x1,3);
#undef DISPLAY
}

int qled_display_negative(int *pins,int c) {  
#define DISPLAY(c,v,i) {if((c % 4) == (v)) digitalWrite(pins[(i)],LOW);else digitalWrite(pins[(i)],HIGH);}
  DISPLAY(c,0,4);
  DISPLAY(c,1,5);
  DISPLAY(c,2,6);
  DISPLAY(c,3,7);
#undef DISPLAY
}

void qled_close(void) {
  close(fd);
}
void qled_close_signal(int i) {
  close(fd);
}



int main(int argc, char **argv) {
  printf("LED CONTROLLER DAEMON START\n");
  atexit(qled_close);
  signal(SIGINT, qled_close_signal);  
  int pins[8] = {0,1,3,4,24,25,27,28};
  //             + + + + -  -  -  -
  // controller
  pin_setup(pins);
  fd = open(argv[1],O_RDWR);
  const unsigned long __size = 16 * sysconf(_SC_PAGESIZE);
  char *buffer = mmap(NULL,__size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
  if(buffer == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }
  bzero(buffer,__size);
  char pos,neg;
  unsigned long i = 0;
  while(1) { // main loop
    for(i %= 4; i < __size; ++ i){
      if (buffer[i]) {
	pos = buffer[i] & 0x0F;
	qled_display_positive(pins,pos);
	qled_display_negative(pins,i);
	delay(1u);
      }
      else break;
    }
  }
  return 0;
}
