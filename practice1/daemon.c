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

#include <wiring.c>

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
  daemon_process();
}

int daemon_process() {
  if(setsid() < 0)
    return -1;
  
}
