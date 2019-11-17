#include "compute.h"

int NumJobsSent = 0;
int NumJobsRec = 0;
int num_threads = 0;
int workCount = 0;

int main(int argc, char *argv[])
{
	signal(SIGINT, ctrl_c_handler);

	if( ! ((argc == 3) || (argc == 2)) )
	{
		fprintf(stderr, "USAGE ERROR: ./compute <Num of Threads> OR ./compute <Num of Threads> -n\n");
                Goodbye();
	}

	int msgid;
  	int n;
	key_t key = ftok("ttobrien", 11);
	if(key == -1)
	{
		fprintf(stderr, "ERROR: Key not produced\n");
                Goodbye();
	}

	msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
	if(msgid == -1)
	{
		msgid = msgget(key, 0);
	}
	if(msgid == -1)
	{
		fprintf(stderr, "ERROR: Message queue id not produced\n");
		Goodbye();
	}
	
	struct msqid_ds ds;
	msgctl(msgid, IPC_STAT, &ds);
	printf("max num of bytes allowed on queue: %lu\n", ds.msg_qbytes); 
	printf("current num of bytes in queue: %lu\n", ds.__msg_cbytes);
	printf("current num of messages in queue: %lu\n", ds.msg_qnum);
	if(argc == 3)
	{
		checkArg3(argv[2]);
		n = 1;
	}
	else
	{
		n = 0;
	}

	num_threads = GetNumOfThreads(argv[1]);
	tpool_t* tm;
	tm = tpool_create(num_threads);
	ComArgs comInfo;// = (ComArgs*) malloc(sizeof(ComArgs));
	comInfo.mqID = &msgid;
	comInfo.nFlag = &n;

	//NO HARDCODING VALS
	/*for (int i = 0; i < 100; i++)
	{
		tpool_add_work(tm, DotProduct, &comInfo);
	}*/

	//http://pages.cs.wisc.edu/~remzi/OSTEP/threads-cv.pdf
	int pthreadRC = 0;
	while(1)
	{
		pthreadRC = pthread_mutex_lock(&workControl);
		if(pthreadRC == -1)
		{
			fprintf(stderr, "ERROR: workControl locking failed\n");
			Goodbye();
		}
		while(tm->working_cnt == num_threads)
		{
			pthreadRC = pthread_cond_wait(&empty, &workControl);
			if(pthreadRC == -1)
			{
				fprintf(stderr, "ERROR: cond_wait failed\n");
				Goodbye();
			}
		}	
		
		tpool_add_work(tm, DotProduct, &comInfo);
		workCount++;
		pthreadRC = pthread_mutex_unlock(&workControl);
		if(pthreadRC == -1)
		{
			fprintf(stderr, "ERROR: workControl unlocking failed\n");
			Goodbye();
		}
	}
	tpool_wait(tm); //do I actually need this?

	return 0;
}

void DotProduct(void* param)
{
	int id = 0, row = 0, col = 0, inner = 0, dp = 0;
	int  n = 0, msgid = 0;
	int rc1 = 0, rc2 = 0, pthreadRC = 0;
	Entry sendBack;
	Msg message;

	ComArgs* comArgs = (ComArgs*) param;

	n = *(comArgs->nFlag);
	msgid = *(comArgs->mqID);

	pthreadRC = pthread_mutex_lock(&lock4);
	if(pthreadRC == -1)
	{
		fprintf(stderr, "ERROR: lock4 locking failed\n");
		Goodbye();
	}
	//printf("tid: %p\n", (void *)pthread_self());
	rc1 = msgrcv(msgid, &message, 104 * sizeof(int), 1, 0);
	if(rc1 == -1)
	{
		printf("ERROR: Message not recieved\n");
		Goodbye();
	}
	NumJobsRec++;
	printf("Recieving job id %d type %ld size %ld\n", message.jobid, message.type, (4 + 2 * message.innerDim) * sizeof(int));
	pthreadRC = pthread_mutex_unlock(&lock4);
	if(pthreadRC == -1)
	{
		fprintf(stderr, "ERROR: lock4 unlocking failed\n");
		Goodbye();
	}

	id = message.jobid;
  	row = message.rowvec;
  	col = message.colvec;
  	inner = message.innerDim;
	for(int i = 0; i < inner; i++)
	{
		dp = dp + message.data[i] * message.data[inner + i];
	}

	sendBack.dotProduct = dp;
	sendBack.type = 2;
	sendBack.jobid = id;
	sendBack.rowvec = row;
	sendBack.colvec = col;

	if (n == 1)
	{
		printf("Sum for cell %d,%d is %d\n", row, col, dp);
	}
	else
	{
		pthreadRC = pthread_mutex_lock(&lock3);
		if(pthreadRC == -1)
		{
			fprintf(stderr, "ERROR: lock3 locking failed\n");
			Goodbye();
		}
		rc2 = msgsnd(msgid, &sendBack, 4 * sizeof(int), 0);
		if(rc1 == -1)
    		{
                	printf("ERROR: Message not sent\n");
		}
		NumJobsSent++;
		printf("Sending job id %d type %ld size %ld (rc=%d)\n", id, sendBack.type, 4 * sizeof(int), rc2);
		pthreadRC = pthread_mutex_unlock(&lock3);
		if(pthreadRC == -1)
		{
			fprintf(stderr, "ERROR: lock3 unlocking failed\n");
			Goodbye();
		}
	}

	pthreadRC = pthread_mutex_lock(&workControl);
	if(pthreadRC == -1)
	{
		fprintf(stderr, "ERROR: workControl locking failed\n");
		Goodbye();
	}
	workCount--; //Critical Section
	pthreadRC = pthread_cond_signal(&empty);
	if(pthreadRC == -1)
	{
		fprintf(stderr, "ERROR: cond_signal failed\n");
		Goodbye();
	}
	pthreadRC = pthread_mutex_unlock(&workControl);
	if(pthreadRC == -1)
	{
		fprintf(stderr, "ERROR: workControl unlocking failed\n");
		Goodbye();
	}

	return;
}

//Source: https://www.geeksforgeeks.org/write-a-c-program-that-doesnt-terminate-when-ctrlc-is-pressed/
void ctrl_c_handler(int sig_num)
{
        signal(SIGINT, ctrl_c_handler);
        printf("Jobs sent %d Jobs Recieved %d\n", NumJobsSent, NumJobsRec);
        fflush(stdout);
}

void Goodbye()
{
	fprintf(stderr, "Program terminating\n");
	exit(0);
}

int GetNumOfThreads(char* arg2)
{
        int num;
	int len = strlen(arg2);
        for(int i = 0; i < len; i++)
	{
		if(! isdigit(arg2[i]))
		{
			fprintf(stderr, "ERROR: Argument 2 should be an integer\n");
	                Goodbye();
		}
	}
        sscanf(arg2, "%d", &num);

        return num;
}


void checkArg3(char* arg3)
{
	if(strlen(arg3) != 2)
	{
		fprintf(stderr, "ERROR: Argument 3 should be \"-n\"\n");
		Goodbye();
	}
	else
	{
		if((arg3[0] != '-') || (arg3[1] != 'n'))
		{
			fprintf(stderr, "ERROR: Argument 3 should be \"-n\"\n");
	                Goodbye();
		}
	}
	return;
}

//delete unnecesaary functions and add stderr protections
static tpool_work_t *tpool_work_create(thread_func_t func, void *arg)
{
    tpool_work_t *work;

    if (func == NULL)
        return NULL;

    work       = (tpool_work_t*) malloc(sizeof(tpool_work_t));
    work->func = func;
    work->arg  = arg;
    work->next = NULL;
    return work;
}

static void tpool_work_destroy(tpool_work_t *work)
{
    if (work == NULL)
        return;
    free(work);
}

static tpool_work_t *tpool_work_get(tpool_t *tm)
{
    tpool_work_t *work;

    if (tm == NULL)
        return NULL;

    work = tm->work_first;
    if (work == NULL)
        return NULL;

    if (work->next == NULL) {
        tm->work_first = NULL;
        tm->work_last  = NULL;
    } else {
        tm->work_first = work->next;
    }

    return work;
}

static void *tpool_worker(void *arg)
{
    tpool_t      *tm = arg;
    tpool_work_t *work;

    while (1) {
        pthread_mutex_lock(&(tm->work_mutex));
        if (tm->stop)
            break;

        if (tm->work_first == NULL)
            pthread_cond_wait(&(tm->work_cond), &(tm->work_mutex));

        work = tpool_work_get(tm);
        tm->working_cnt++;
        pthread_mutex_unlock(&(tm->work_mutex));

        if (work != NULL) {
            work->func(work->arg);
            tpool_work_destroy(work);
        }

        pthread_mutex_lock(&(tm->work_mutex));
        tm->working_cnt--;
        if (!tm->stop && tm->working_cnt == 0 && tm->work_first == NULL)
            pthread_cond_signal(&(tm->working_cond));
        pthread_mutex_unlock(&(tm->work_mutex));
    }

    tm->thread_cnt--;
    pthread_cond_signal(&(tm->working_cond));
    pthread_mutex_unlock(&(tm->work_mutex));
    return NULL;
}

tpool_t *tpool_create(size_t num)
{
    tpool_t   *tm;
    pthread_t  thread;
    size_t     i;

    if (num == 0)
        num = 2;

    tm             = calloc(1, sizeof(*tm));
    tm->thread_cnt = num;

    pthread_mutex_init(&(tm->work_mutex), NULL);
    pthread_cond_init(&(tm->work_cond), NULL);
    pthread_cond_init(&(tm->working_cond), NULL);

    tm->work_first = NULL;
    tm->work_last  = NULL;

    for (i=0; i<num; i++) {
        pthread_create(&thread, NULL, tpool_worker, tm);
        pthread_detach(thread);
    }
    tm->working_cnt = 0;
    return tm;
}

void tpool_destroy(tpool_t *tm)
{
    tpool_work_t *work;
    tpool_work_t *work2;

    if (tm == NULL)
        return;

    pthread_mutex_lock(&(tm->work_mutex));
    work = tm->work_first;
    while (work != NULL) {
        work2 = work->next;
        tpool_work_destroy(work);
        work = work2;
    }
    tm->stop = true;
    pthread_cond_broadcast(&(tm->work_cond));
    pthread_mutex_unlock(&(tm->work_mutex));

    tpool_wait(tm);

    pthread_mutex_destroy(&(tm->work_mutex));
    pthread_cond_destroy(&(tm->work_cond));
    pthread_cond_destroy(&(tm->working_cond));

    free(tm);
}

bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg)
{
    tpool_work_t *work;

    if (tm == NULL)
        return false;

    work = tpool_work_create(func, arg);
    if (work == NULL)
        return false;

    pthread_mutex_lock(&(tm->work_mutex));
    if (tm->work_first == NULL) {
        tm->work_first = work;
        tm->work_last  = tm->work_first;
    } else {
        tm->work_last->next = work;
        tm->work_last       = work;
    }

    pthread_cond_broadcast(&(tm->work_cond));
    pthread_mutex_unlock(&(tm->work_mutex));

    return true;
}

void tpool_wait(tpool_t *tm)
{
    if (tm == NULL)
        return;

    pthread_mutex_lock(&(tm->work_mutex));
    while (1) {
        if ((!tm->stop && tm->working_cnt != 0) || (tm->stop && tm->thread_cnt != 0)) {
            pthread_cond_wait(&(tm->working_cond), &(tm->work_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tm->work_mutex));
}
