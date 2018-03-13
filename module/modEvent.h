#define GET_SIZE 0
#define SET_PID 1
#define EXEC_EVENT 3

struct eventUser{
  pid_t pid;
  char type[20];
  unsigned long sec;
  unsigned long nsec;
};
