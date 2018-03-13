// escribe un evento en el buffer
void wEvent(char type, struct task_struct* task){

  task->buffEvent[task->posEvent].type = type;

  struct timespec time;
  getnstimeofday(&time);
  task->buffEvent[task->posEvent].sec = time.tv_sec;
  task->buffEvent[task->posEvent].nsec = time.tv_nsec;

  task->posEvent++;
  if(task->posEvent >= MAX_SIZE){
    task->posEvent = 0;
    task->fullEvent = 1;
  }
}
