#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <event.h>
#include "modEvent.h"

void sort (struct eventUser arr[], int length){
	 	int j,i;
		struct eventUser temp;

	for (i = 0; i < length; i++){
		j = i;

		while (j > 0 && (arr[j].sec < arr[j-1].sec || (arr[j].sec == arr[j-1].sec && arr[j].nsec < arr[j-1].nsec))){
				temp = arr[j];
				arr[j] = arr[j-1];
				arr[j-1] = temp;
				j--;
				}
		}
}

int main(int argc, char *argv[])
{
	int i,j,fd,size,ret;
	pid_t pid[100];
	struct event res[MAX_SIZE];
	struct eventUser resUser[MAX_SIZE];
	struct eventUser resFinal[MAX_SIZE*32];
	int resFinalPos = 0;

	if(argc < 2){
		printf("Invalid number of parameters\n");
		return -1;
	}

	fd = open("/dev/modEvent", O_RDWR);

	for (i = 1; i < argc; i++){
		pid[i-1] = atoi(argv[i]);
	}

	for (i = 0; i < argc - 1; i++){
		kill(pid[i],SIGSTOP);
	}

	for (i = 0; i < argc-1; i++){
		ioctl(fd,SET_PID,&pid[i]);
		ioctl(fd,GET_SIZE,&size);
		ret = read(fd,&res,size);
		if(ret<0)
			printf("error\n");
		else{
			for(j = 0; j < size; j++){
				resFinal[resFinalPos].pid = pid[i];
				switch (res[j].type) {
					case 0:
						strcpy(resFinal[resFinalPos].type, "Starting execution");
					break;
					case 1:
						strcpy(resFinal[resFinalPos].type, "Ending execution");
					break;
					case 2:
						strcpy(resFinal[resFinalPos].type, "Awakening process");
					break;
				}
				resFinal[resFinalPos].sec = res[j].sec;
				resFinal[resFinalPos].nsec = res[j].nsec;

				resFinalPos++;
			}
		}
	}

	pid[argc-1] = -1;
	ioctl(fd,SET_PID,&pid[argc-1]);
	ioctl(fd,GET_SIZE,&size);
	ret = read(fd,&resUser,size);
	if(ret<0)
		printf("error\n");
	else{
		for(j = 0; j < size; j++){
			ret = 0;
			for(i = 0; i < argc - 1; i++){
				if(pid[i] == resUser[j].pid)ret = 1;
			}
			if(ret){
				resFinal[resFinalPos].pid = resUser[j].pid;
				strcpy(resFinal[resFinalPos].type, resUser[j].type);
				resFinal[resFinalPos].sec = resUser[j].sec;
				resFinal[resFinalPos].nsec = resUser[j].nsec;

				resFinalPos++;
			}
		}
	}

	sort(resFinal,resFinalPos);

	for (i = 0; i < resFinalPos; i++){
		printf("%d,%s,%lu%lu\n", resFinal[i].pid,resFinal[i].type,resFinal[i].sec,resFinal[i].nsec);
	}

	for (i = 0; i < argc-1; i++){
		kill(pid[i],SIGCONT);
	}

	close(fd);
	return 0;
}
