//
// Created by Tommy O'Brien on November 14, 2019
//

#ifndef CS300Project_PACKAGE_H
#define CS300Project_PACKAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/ipc.h> 
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


/*
 * This function puts puts a type 1 Msg on the message queue.
 * Then it recieves a type 2 Entry from the message queue.
 * The recieved messages are used to populate the product matrix.
 *
 * The use of struct msqid_ds with msgctl was adapted from the msgctl man page.
 * http://man7.org/linux/man-pages/man2/msgctl.2.html
 *
 * The implementation of the message queue is adapted from
 * https://www.geeksforgeeks.org/ipc-using-message-queues/
 */
void* ProducerSendAndRecieve(void*);

/*
 * Overriding SIGINT (Ctrl-C) to print how many jobs have been sent and recieved.
 *
 * Adapted from: https://www.geeksforgeeks.org/write-a-c-program-that-doesnt-terminate-when-ctrlc-is-pressed/
 */
void CtrlC(int);

/*
 * This function recieves the fifth command line argument as a c string.
 * Then if it is an integer, it is returned as an int.
 */
int GetSecs(char*);

/*
 * This function is called when a runtime error occurs, and when called it notifies
 * the user that the program is terminating and then proceeds to do so.
 */
void Goodbye();

#endif //CS300Project_PACKAGE_H
