//Author: Tommy O'Brien
//Start date: 10-26-2019

#include "package.h"

int NumJobsSent = 0;
int NumJobsRec = 0;

int main(int argc, char *argv[])
{
	signal(SIGINT, ctrl_c_handler);

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

	fscanf(matrixFile1, " %d", &m1RowsNum);
	fscanf(matrixFile1, " %d\n", &m1ColsNum);
	fscanf(matrixFile2, " %d", &m2RowsNum);
  	fscanf(matrixFile2, " %d\n", &m2ColsNum);

	assert(m1ColsNum == m2RowsNum);
	assert((m1RowsNum >= 1) && (m1RowsNum <= 50));
	assert((m2RowsNum >= 1) && (m2RowsNum <= 50));
	assert((m1ColsNum >= 1) && (m1ColsNum <= 50));
	assert((m2ColsNum >= 1) && (m2ColsNum <= 50));

	//stackoverflow 2128728 allocate matrix in C

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


	int** mOut = (int **) malloc(m1RowsNum * sizeof(int*));
        for(int a = 0; a < m1RowsNum; a++)
	{
		mOut[a] = (int*) malloc(m2ColsNum * sizeof(int));
	}
	int NumThreads = m1RowsNum * m2ColsNum; //number of entries that will exist in the output matrix

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

	PreMsg* outgoing = (PreMsg*) malloc(NumThreads * sizeof(PreMsg));//stack over flow 10468128 how do you make an array of structs in C?

	pthread_t threads[NumThreads];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int createRC = 0;
	for(int i = 0; i < NumThreads; i++)
	{
   		outgoing[i].jobidP = i;
		outgoing[i].mqidP = &msgid;
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

	int rcJoin;

	for(int j = 0; j < NumThreads; j++)
	{
		rcJoin = pthread_join(threads[j], NULL);
		if(rcJoin < 0)
		{
			fprintf(stderr, "ERROR: Thread %d not joined correctly\n", j);
			Goodbye();
		}
	}

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
	for(int a = 0; a < m1RowsNum; a++)
	{
		free(mOut[a]);
	}
	free(mOut);
	printf("\n\n\n");
	fclose(outputFile);
		
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
	PreMsg* info = (PreMsg*)infoVoid;

	int msgid = *(info->mqidP);

	Msg message;
	message.type = 1;
	message.jobid = info->jobidP;
	message.rowvec = info->jobidP / *(info->m2C);
	message.colvec = info->jobidP % *(info->m2C);
	message.innerDim = *(info->m1C);

	//printf("type: %ld, jobid: %d, rowvec: %d, colvec: %d, innerdim: %d\n", message.type, message.jobid, message.rowvec, message.colvec, message.innerDim);

	for(int a = 0; a < message.innerDim; a++)
        {
        	message.data[a] = info->m1[info->jobidP / *(info->m2C)][a];
		message.data[a + message.innerDim] = info->m2[a][info->jobidP % *(info->m2C)];
       	}

  	int pthreadRC = 0;

	//printf("msgid: %d\n", msgid);
	pthreadRC = pthread_mutex_lock(&lock1);
	if(pthreadRC == -1)
  	{
		fprintf(stderr, "ERROR: lock1 locking failed\n");
		Goodbye();
	}
	int rc1 = msgsnd(msgid, &message, (4 + 2 * message.innerDim) * sizeof(int), 0);
	if(rc1 == -1)
	{
		fprintf(stderr, "ERROR: msgsnd failed\n");
		Goodbye();
	}
	NumJobsSent++;
	printf("Sending job id %d type %ld size %ld (rc=%d)\n", message.jobid, message.type, (4 + 2 * message.innerDim) * sizeof(int), rc1);
	pthreadRC = pthread_mutex_unlock(&lock1);
	if(pthreadRC == -1)
	{
		fprintf(stderr, "ERROR: lock1 unlocking failed\n");
		Goodbye();
	}

	Entry entry;
	pthreadRC = pthread_mutex_lock(&lock2);
	if(pthreadRC == -1)
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
	printf("Recieving job id %d type %ld size %ld\n", entry.jobid, entry.type, 4 * sizeof(int));
	NumJobsRec++;
	pthreadRC = pthread_mutex_unlock(&lock2);
	if(pthreadRC == -1)
	{
		fprintf(stderr, "ERROR: lock2 unlocking failed\n");
		Goodbye();
	}

	info->m3[entry.rowvec][entry.colvec] = entry.dotProduct;

  return NULL;
}


int GetSecs(char* arg5)
{
	int i, secs;
	int len = strlen(arg5);
	for(i = 0; i < len; i++)
		assert(isdigit(arg5[i]));
	sscanf(arg5, "%d", &secs);
	return secs;
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

