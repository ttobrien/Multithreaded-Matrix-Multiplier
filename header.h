#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

pthread_mutex_t lock1;

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

typedef struct PreQueueMessage{
 long typeP;
 int jobidP;
 int rowvecP;
 int colvecP;
int innerDimP;
 int dataP[100];
 int mqidP;
} PreMsg;

