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

static int fd;

int main() {
  printf("LED CONTROLLER DAEMON START\n");
  int pid = fork();
  if(pid != 0) {
    printf("ERR: fail fork\n");
    return pid > 0;
  }
  if(setsid() < 0) {
    printf("ERR: fail setsid\n");
    return 2;
  }  
  struct sigaction act;   
  act.sa_handler=SIG_IGN;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGHUP,&act,NULL)<0){
    printf("sigaction error.");
    return 3;
  }
  pid = fork();
  if (pid != 0) {
    printf("ERR: fail fork fork\n");
    return pid > 0;
  }    
  chdir("/");
  umask(0);
  for(i=0;i<NOFILE;i++)
    close(i);
  return daemon_process();
}


int pin_setup(int* pins) {
  wiringPiSetup();
  for(i = 0; i< 8; ++i)
    pinMode(pins[i],OUTPUT);
}



int qled_display_positive(int *pins,char c) {
#define DISPLAY(c,v,i) {if((i) & (v)) digitalWrite(pins[(i)],HIGH);else digitalWrite(pins[(i)],LOW);}
  DISPLAY(c,0x8,0);
  DISPLAY(c,0x4,1);
  DISPLAY(c,0x2,2);
  DISPLAY(c,0x1,3);
#undef DISPLAY
}

int qled_display_negative(int *pins,char c) {  
#define DISPLAY(c,v,i) {if((i % 4) == (v)) digitalWrite(pins[(i)],LOW);else digitalWrite(pins[(i)],HIGH);}
  DISPLAY(c,0,4);
  DISPLAY(c,1,5);
  DISPLAY(c,2,6);
  DISPLAY(c,3,7);
#undef DISPLAY
}

void qled_close(void) {
  close(fd);
}
void qled_close_signal(int) {
  close(fd);
}


int daemon_process() {
  if(setsid() < 0)
    return -1;
  atexit(qled_close);
  signal(SIGINT, qled_close_signal);  
  int pins[8] = {0,1,3,4,24,25,27,28};
  //             + + + + -  -  -  -
  // controller
  pin_setup(pins);
  char buffer[1024];
  char *cur;
  char pos,neg;
  fd = open("/dev/qled",O_RDWR);
  while(1) { // main loop
    read(fd,buf,1024);
    for(cur = buffer; cur < buffer + 1024; ++ cur){
      if (*cur) {
	pos = *cur & 0x0F;
	neg = (*cur & 0xF0) >> 4;
	qled_display_positive(pins,pos);
	qled_display_negative(pins,neg);
	delay(10);
      }
      else break;
    }
  }
}

