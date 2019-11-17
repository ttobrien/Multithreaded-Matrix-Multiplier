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
#include <stdbool.h>

pthread_cond_t empty = PTHREAD_COND_INITIALIZER; //used for controlling the number of workers
pthread_mutex_t workControl = PTHREAD_MUTEX_INITIALIZER; //used in controlling the number of workers
pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER; //for global variable int NumJobsSent in compute.c
pthread_mutex_t lock4 = PTHREAD_MUTEX_INITIALIZER; //for global variable int NumJobsRec in compute.c

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

typedef struct ComputeInfoForThread{
	int* mqID;
	int* nFlag;
} ComArgs;

typedef void (*thread_func_t)(void *arg);

typedef struct tpool_work {
    thread_func_t      func;
    void              *arg;
    struct tpool_work *next;
} tpool_work_t;

typedef struct tpool {
    tpool_work_t    *work_first;
    tpool_work_t    *work_last;
    pthread_mutex_t  work_mutex;
    pthread_cond_t   work_cond;
    pthread_cond_t   working_cond;
    size_t           working_cnt;
    size_t           thread_cnt;
    bool             stop;
} tpool_t;

/*
 * This function receives a type 1 Msg from the message queue.
 * The dot product is computed for (rowvec, colvec) with the data array.
 * Then a type 2 Entry is placed onto the message queue.
 *
 * The implementation of the message queue is adapted from
 * https://www.geeksforgeeks.org/ipc-using-message-queues/
 */
void DotProduct(void*);

/*
 * This funtction is overriding SIGINT (Ctrl-C) to print how many jobs have been sent and recieved.
 *
 * Adapted from: https://www.geeksforgeeks.org/write-a-c-program-that-doesnt-terminate-when-ctrlc-is-pressed/
 */
void CtrlC(int);

/*
 * This function is called when a runtime error occurs, and when called it notifies
 * the user that the program is terminating and then proceeds to do so.
 */
void Goodbye();

/*
 * This function recieves the second command line argument as a c string.
 * Then if it is an integer, it is returned as an int.
 */
int GetNumOfThreads(char*);

/*
 * This function ensures that if a third command line argument is entered that it
 * is a proper call for the non-rteurn switch "-n" to be activated.
 */
void checkArg3(char*);

/*
 * This function is used to allocate space for and intialize a function
 * call for the thread pool to process.
 *
 * Adapted from John Nachtimwald's "Thread Pool in C"
 * https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 */
static tpool_work_t *tpool_work_create(thread_func_t, void*);

/*
 * This function is used to de allocate space from a function call that the thread
 * pool has processed.
 *
 * Adapted from John Nachtimwald's "Thread Pool in C"
 * https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 */
static void tpool_work_destroy(tpool_work_t*);

/*
 * This function pulls  function call off of the queue and returns it for the
 * thread pool to process.
 *
 * Adapted from John Nachtimwald's "Thread Pool in C"
 * https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 */
static tpool_work_t *tpool_work_get(tpool_t*);

/*
 * This function is used to manage the threads in the thread pool.
 *
 * Adapted from John Nachtimwald's "Thread Pool in C"
 * https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 */
static void *tpool_worker(void*);

/*
 * This function is used to create the number of detached threads that was
 * received from the command tline for the thread pool to utilize.
 *
 * Adapted from John Nachtimwald's "Thread Pool in C"
 * https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 */
tpool_t *tpool_create(size_t);

/*
 * This function is called from main to call DotProduct when there is a free
 * thread in the thread pool.
 *
 * Adapted from John Nachtimwald's "Thread Pool in C"
 * https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 */
bool tpool_add_work(tpool_t*, thread_func_t, void*);

#endif CS300Project_PACKAGE_H
