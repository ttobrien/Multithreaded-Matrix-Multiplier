//
// Created by Tommy O'Brien on October 26, 2019
//

#include "package.h"

int numJobsSent = 0;
int numJobsRec = 0;

int main(int argc, char *argv[])
{
	signal(SIGINT, CtrlC); //set interrupt

	if( ! ((argc == 4) || (argc == 5)) )
   	{
    		fprintf(stderr, "USAGE ERROR: ./package  <matrix 1 file> <matrix 2 file> <output matrix data file> <secs between thread creation>\n");
		fprintf(stderr, "OR\t ./package  <matrix 1 file> <matrix 2 file> <output matrix data file>\n");
    		Goodbye();
    	}


	FILE* matrixFile1 = NULL;
	FILE* matrixFile2 = NULL;
	FILE* outputFile = NULL;
	matrixFile1 = fopen(argv[1], "r");
	matrixFile2 = fopen(argv[2], "r");
	outputFile = fopen(argv[3], "w");

	if((matrixFile1 == NULL) || (matrixFile2 == NULL) || (outputFile == NULL))
	{
		fprintf(stderr, "ERROR: File not opened properly");
		Goodbye();
	}


	int m1RowsNum = 0, m1ColsNum = 0, m2RowsNum = 0, m2ColsNum = 0;
	int secs;
  	if(argc == 5)
	{
		secs = GetSecs(argv[4]);
	}
	else
	{
		secs = 0;
	}
	
	//getting matrix dimensions out of the files
	fscanf(matrixFile1, " %d", &m1RowsNum);
	fscanf(matrixFile1, " %d\n", &m1ColsNum);
	fscanf(matrixFile2, " %d", &m2RowsNum);
  	fscanf(matrixFile2, " %d\n", &m2ColsNum);
	
	//ensuring matrix dimensions meet project specifications
	assert(m1ColsNum == m2RowsNum);
	assert((m1RowsNum >= 1) && (m1RowsNum <= 50));
	assert((m2RowsNum >= 1) && (m2RowsNum <= 50));
	assert((m1ColsNum >= 1) && (m1ColsNum <= 50));
	assert((m2ColsNum >= 1) && (m2ColsNum <= 50));
	
	//Allocating space for both matrices and reading them from the files to the matrices
	int** matrix1 = (int **) malloc(m1RowsNum * sizeof(int*));
	for(int a = 0; a < m1RowsNum; a++)
	{
		matrix1[a] = (int*) malloc(m1ColsNum * sizeof(int));
	}
	int** matrix2 = (int **) malloc(m2RowsNum * sizeof(int*));
        for(int a = 0; a < m2RowsNum; a++)
	{
		matrix2[a] = (int*) malloc(m2ColsNum * sizeof(int));
	}
	for(int i = 0; i < m1RowsNum; i++)
	{
		for(int j = 0; j < m1ColsNum; j++)
		{
			fscanf(matrixFile1, " %d", &matrix1[i][j]);
		}
	}
	for(int i = 0; i < m2RowsNum; i++)
	{
	  	 for(int j = 0; j < m2ColsNum; j++)
		 {
			 fscanf(matrixFile2, " %d", &matrix2[i][j]);
		 }
	}
	fclose(matrixFile1);
	fclose(matrixFile2);

	//matrix that will store the product matrix
	int** mOut = (int **) malloc(m1RowsNum * sizeof(int*));
        for(int a = 0; a < m1RowsNum; a++)
	{
		mOut[a] = (int*) malloc(m2ColsNum * sizeof(int));
	}

	int NumThreads = m1RowsNum * m2ColsNum; //number of entries that will exist in the output matrix
	
	//Creating message queue or at least getting the id of an already created message queue
	int msgid;
	key_t key = ftok("ttobrien", 11);
	if(key < 0)
	{
		fprintf(stderr, "ERROR: Key not made\n");
		Goodbye();
	}
	msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
	if(msgid == -1)
	{
		msgid = msgget(key, 0);
	}
	if(msgid < 0)
	{
		fprintf(stderr, "ERROR: Message queue id not made\n");
		Goodbye();
	}

	//all of the arguments that will need to be sent by pthread_create
	PreMsg* outgoing = (PreMsg*) malloc(NumThreads * sizeof(PreMsg));

	pthread_t threads[NumThreads];
	pthread_attr_t attr;//used to make threads joinable
	pthread_attr_init(&attr);
	int createRC = 0;
	for(int i = 0; i < NumThreads; i++)
	{

   		outgoing[i].jobidP = i;
		outgoing[i].mqidP = &msgid; //sending pointers to reduce amount of bytes being sent
		outgoing[i].m2C = &m2ColsNum;
		outgoing[i].m1C = &m1ColsNum;
		outgoing[i].m1 = matrix1;
		outgoing[i].m2 = matrix2;
		outgoing[i].m3 = mOut;

		createRC = pthread_create(&threads[i], &attr, ProducerSendAndRecieve, &outgoing[i]);

		if(createRC == -1)
		{
			fprintf(stderr, "ERROR: pthread_create failed for thread %d\n", i);
			Goodbye();
		}
		sleep(secs);
	}

	//all threads must be joined to ensure full completion of matrix multiplication
	int rcJoin = 0;
	for(int j = 0; j < NumThreads; j++)
	{
		rcJoin = pthread_join(threads[j], NULL);
		if(rcJoin < 0)
		{
			fprintf(stderr, "ERROR: Thread %d not joined correctly\n", j);
			Goodbye();
		}
	}
	
	//printing product matrix to the screen in a formatted manner and to the specified output file in order seperated by spaces
	printf("\n\nResulting Matrix:\n\n");
	for(int a = 0; a < m1RowsNum; a++)
	{
		for(int b = 0; b < m2ColsNum; b++)
		{
			printf("%4d ", mOut[a][b]);
			fprintf(outputFile, "%d ", mOut[a][b]);
		}
		printf("\n");
	}
	printf("\n\n\n");
        fclose(outputFile);

	//freeing all heap allocated memory
	for(int a = 0; a < m1RowsNum; a++)
	{
		free(mOut[a]);
	}
	free(mOut);
	free(outgoing);
  	for(int a = 0; a < m1RowsNum; a++)
        {
		free(matrix1[a]);
	}
  	free(matrix1);
  	for(int a = 0; a < m2RowsNum; a++)
	{
		free(matrix2[a]);
	}
  	free(matrix2);

	return 0;
}

void* ProducerSendAndRecieve(void* infoVoid)
{
	int rcPthread = 0;
	struct msqid_ds ds;
	Entry entry;
	
	//using pointers to limit number of bytes allocated to heap and this thread's stack
	PreMsg* info = (PreMsg*)infoVoid;	
	int msgid = *(info->mqidP);
	Msg message;
	message.type = 1;
	message.jobid = info->jobidP;
	message.rowvec = info->jobidP / *(info->m2C);
	message.colvec = info->jobidP % *(info->m2C);
	message.innerDim = *(info->m1C);
	for(int a = 0; a < message.innerDim; a++)
        {
        	message.data[a] = info->m1[info->jobidP / *(info->m2C)][a];
		message.data[a + message.innerDim] = info->m2[a][info->jobidP % *(info->m2C)];
       	}
	
	//calls of msgsnd and incrementing of numJobsSent are syncronized
  	rcPthread = pthread_mutex_lock(&lock1);
	if(rcPthread == -1)
  	{
		fprintf(stderr, "ERROR: lock1 locking failed\n");
		Goodbye();
	}
	
	msgctl(msgid, IPC_STAT, &ds);
	int sizeOfMessage = (4 + 2 * message.innerDim) * sizeof(int);
	
	//ensure the byte limit on the message queue will not be surpassed
	while((sizeOfMessage + ds.__msg_cbytes) > ds.msg_qbytes)
	{
		pthread_cond_wait(&cond, &lock1);
		msgctl(msgid, IPC_STAT, &ds);
	}

	int rc1 = msgsnd(msgid, &message, sizeOfMessage, 0);
	if(rc1 == -1)
	{
		fprintf(stderr, "ERROR: msgsnd failed\n");
		Goodbye();
	}
	numJobsSent++;
	printf("Sending job id %d type %ld size %ld (rc=%d)\n", message.jobid, message.type, (4 + 2 * message.innerDim) * sizeof(int), rc1);
	rcPthread = pthread_mutex_unlock(&lock1);
	if(rcPthread == -1)
	{
		fprintf(stderr, "ERROR: lock1 unlocking failed\n");
		Goodbye();
	}

	//calls of msgrcv and incrementing of numJobsRec are synchronized
	rcPthread = pthread_mutex_lock(&lock2);
	if(rcPthread == -1)
	{
		fprintf(stderr, "ERROR: lock2 locking failed\n");
		Goodbye();
	}
	int rc2 = msgrcv(msgid, &entry, 4 * sizeof(int), 2, 0);
	if(rc2 == -1)
	{
		fprintf(stderr, "ERROR: msgrcv failed\n");
		Goodbye();
	}
	pthread_cond_broadcast(&cond);//wake all threads that are waiting to send a message
	printf("Receiving job id %d type %ld size %ld\n", entry.jobid, entry.type, 4 * sizeof(int));
	numJobsRec++;
	rcPthread = pthread_mutex_unlock(&lock2);
	if(rcPthread == -1)
	{
		fprintf(stderr, "ERROR: lock2 unlocking failed\n");
		Goodbye();
	}

	//populate product matrix here to limit number of bytes allocated to heap by not passing a struct back
	info->m3[entry.rowvec][entry.colvec] = entry.dotProduct;

  	return NULL;
}


int GetSecs(char* arg5)
{
	int i, secs;
	int len = strlen(arg5);
	for(i = 0; i < len; i++)
		assert(isdigit(arg5[i]));
	sscanf(arg5, "%d", &secs);//turning c string into integer if safe
	return secs;
}


void CtrlC(int sig_num)
{
        signal(SIGINT, CtrlC); //resets interrupt
        printf("Jobs Sent %d Jobs Received %d\n", numJobsSent, numJobsRec);
        fflush(stdout);
}

void Goodbye()
{
        fprintf(stderr, "Program terminating\n");
        exit(0);
}
