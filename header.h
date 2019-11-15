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
pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER; //for global variable int NumJobsSent in compute.c
pthread_mutex_t lock4 = PTHREAD_MUTEX_INITIALIZER; //for global variable int NumJobsRec in compute.c

int ALLDONE = 0;

typedef struct Computed{
        long type;
        int jobid;
        int rowvec;
        int colvec;
        int dotProduct;
} Entry;

typedef struct QueueMessage{
 long type;
 int jobid;
 int rowvec;
 int colvec;
int innerDim;
 int data[100];
} Msg;

/*typedef struct PreQueueMessage{
 long typeP;
 int jobidP;
 int rowvecP;
 int colvecP;
int innerDimP;
 int dataP[100];
 int mqidP;
} PreMsg;*/

typedef struct PackageInfoForThread{
	int jobidP;
	int* mqidP;
	int* m2C;
	int* m1C;
	int** m1;
	int** m2;
	int** m3;
} PreMsg;

typedef struct RE{
	int row;
	int col;
	int dp;
} ReturnEntry;

typedef struct ComputeInfo
{
	int* mqID;
	int* nFlag;
} ComArgs;


