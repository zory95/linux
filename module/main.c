#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <pthread.h>
#include "modEvent.h"

#define NTHREAD 3
int fd;

unsigned long long takeTime(){
  struct timeval time;
  gettimeofday(&time, NULL);
  return time.tv_sec * 1000000ULL + time.tv_usec;
}
void *thr(void *ptr){
  unsigned long long next = takeTime(), period = 3000, dif;

  while(1){
    double x[1000],y[1000],z[1000];
    int i;
    for(i = 0; i<1000; i++){
      if(i==500)ioctl(fd,EXEC_EVENT,"mitad del bucle");
      x[i]=5.0*y[i]+z[i];
      y[i]=5.0*x[i]+x[i];
    }

    next += period;
    dif = next - takeTime();
    if(dif<0)dif=0;
    usleep(dif);
  }
  return NULL;
}

int main ( )
{
  pthread_t t[NTHREAD];
  int i;

  fd = open("/dev/modEvent", O_RDWR);
  if(fd < 0){
    printf("modEvent not installed.\n");
    return -1;
  }

  for(i = 0; i < NTHREAD; i++)
    if(pthread_create(&t[i],NULL,thr,(void*)i)){
      fprintf(stderr, "Error al crear el thread");
      return -1;
    }

  for(i = 0; i < NTHREAD; i++)
    pthread_join(t[i],NULL);

  close(fd);

  return 0;
}
