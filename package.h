//
// Created by Tommy O'Brien on November 14, 2019
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/ipc.h> //SOURCE: https://www.geeksforgeeks.org/ipc-using-message-queues/
#include <sys/msg.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER; //for global variable int NumJobsSent in package.c
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER; //for global variable int NumJobsRec in package.c
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;  //for preventing putting too many bytes on the message queue

typedef struct QueueMessage1{
  long type;
  int jobid;
  int rowvec;
  int colvec;
  int innerDim;
  int data[100];
} Msg;

typedef struct QueueMessage2{
  long type;
  int jobid;
  int rowvec;
  int colvec;
  int dotProduct;
} Entry;

typedef struct PackageInfoForThread{
	int jobidP;
	int* mqidP;
	int* m2C;
	int* m1C;
	int** m1;
	int** m2;
	int** m3;
} PreMsg;

void* ProducerSendAndRecieve(void*);
void CtrlC(int);
int GetSecs(char*);
void Goodbye();
