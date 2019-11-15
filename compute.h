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

pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t workControl = PTHREAD_MUTEX_INITIALIZER;
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

void DotProduct(void*);
void ctrl_c_handler(int);
void Goodbye();
int GetNumOfThreads(char*);
void checkArg3(char*);
static tpool_work_t *tpool_work_create(thread_func_t, void*);
static void tpool_work_destroy(tpool_work_t*);
static tpool_work_t *tpool_work_get(tpool_t*);
static void *tpool_worker(void*);
tpool_t *tpool_create(size_t);
void tpool_destroy(tpool_t*);
bool tpool_add_work(tpool_t*, thread_func_t, void*);
void tpool_wait(tpool_t*);
