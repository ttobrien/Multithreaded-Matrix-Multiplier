#include "header.h"

void* DotProduct(void*);
int NumJobsSent = 0;
int NumJobsRec = 0;

//Source: https://www.geeksforgeeks.org/write-a-c-program-that-doesnt-terminate-when-ctrlc-is-pressed/
void ctrl_c_handler(int sig_num)
{
        signal(SIGINT, ctrl_c_handler);
        printf("Jobs sent %d Jobs Recieved %d\n", NumJobsSent, NumJobsRec);
        fflush(stdout);
}

void goodbye()
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
	                goodbye();
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
		goodbye();
	}
	else
	{
		if((arg3[0] != '-') || (arg3[1] != 'n'))
		{
			fprintf(stderr, "ERROR: Argument 3 should be \"-n\"\n");
	                goodbye();
		}
	}
	return;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, ctrl_c_handler);	

	if( ! ((argc == 3) || (argc == 2)) )
	{
		fprintf(stderr, "USAGE ERROR: ./compute <Num of Threads> OR ./compute <Num of Threads> -n\n");
                goodbye();
	}


	
//	ComArgs comInfo;// = (ComArgs*) malloc(sizeof(ComArgs));

/*	if(comInfo == NULL)
	{
		fprintf(stderr, "ERROR: Heap not allocated for comInfo\n");
                goodbye();
	}
*/	
	int msgid;
  	int n;      
	key_t key = ftok("ttobrien", 11);
	if(key == -1)
	{
		fprintf(stderr, "ERROR: Key not produced\n");
                goodbye();
	}

        msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
        if(msgid == -1)
                msgid = msgget(key, 0);
	if(msgid == -1)
	{
		fprintf(stderr, "ERROR: Message queue id not produced\n");
		goodbye();
	}
//	comInfo.mqID = msgid;
//	printf("mqid: %d\n", comInfo.mqID);	

	if(argc == 3)
	{
		checkArg3(argv[2]);
		n = 1;
		//comInfo.nFlag = 1;
	}
	else
	{
		n = 0;
		//comInfo.nFlag = 0;	
	}
	
	int numOfThreads = GetNumOfThreads(argv[1]);
	pthread_t thread[numOfThreads];
	pthread_attr_t attr;
        pthread_attr_init(&attr);
	ComArgs* comInfo = (ComArgs *)malloc(numOfThreads * sizeof(ComArgs));
	for(int i = 0; i < numOfThreads; i++)
	{
		comInfo[i].tid = i;
		comInfo[i].mqID = &msgid;
		comInfo[i].nFlag = &n;
		int rcTC = pthread_create(&thread[i], &attr, DotProduct, &comInfo[i]);
		if(rcTC == -1)
		{
			fprintf(stderr, "ERROR: Thread not created\n");
			goodbye();
		}
	}

	for(int j = 0; j < numOfThreads; j++)
	{
		pthread_join(thread[j], NULL);
	}
	
	return 0;
}

void* DotProduct(void* param)
{
	int id = 0, row = 0, col = 0, inner = 0, dp = 0;
	int  n = 0, msgid = 0;
	int rc1 = 0, rc2 = 0;
	Entry sendBack;
	Msg message;
	
	ComArgs* comArgs = (ComArgs*) param;
//	printf("msgid rec: %d\n", comArgs->mqID);
	n = *(comArgs->nFlag);
	msgid = *(comArgs->mqID);
	
	//PROBLEM: NEED BETTER THREAD POOL METHOD 1 THREAD DOES ALL OF THE WORK
	//printf("tid: %d\n", comArgs->tid);	
	//fflush(stdout);
	while(1)
	{
	
	dp = 0;

	pthread_mutex_lock(&lock4);
//	printf("tid: %d\n", comArgs->tid);
	rc1 = msgrcv(msgid, &message, 104 * sizeof(int), 1, 0);
	if(rc1 == -1)
	{
		printf("ERROR: Message not recieved\n");
		goodbye();
	}
	NumJobsRec++;
	printf("Recieving job id %d type %ld size %ld\n", message.jobid, message.type, (4 + 2 * message.innerDim) * sizeof(int));
	pthread_mutex_unlock(&lock4);
	
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
		pthread_mutex_lock(&lock3);
		rc2 = msgsnd(msgid, &sendBack, 4 * sizeof(int), 0);
		if(rc1 == -1)
        	{
                	printf("ERROR: Message not sent\n");
		}
		NumJobsSent++;
		printf("Sending job id %d type %ld size %ld (rc=%d)\n", id, sendBack.type, 4 * sizeof(int), rc2);
		pthread_mutex_unlock(&lock3);
	}
	
	}

	return 0;
}
