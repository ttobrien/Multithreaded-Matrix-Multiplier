//Author: Tommy O'Brien
//Start date: 10-26-2019

//QUESTIONS
//makefile with c11
//assume proper format and all integers input
//is secs an int

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

void* ProducerSend(void*);
int GetSecs(char*);
void InitTest(int**, int**, FILE*, int, int, int, int, int);


int NumJobsSent = 0;
int NumJobsRec = 0;


int main(int argc, char *argv[])
{
	
	if(argc != 5)
	{
		printf("\nERROR: 5 arguments expected\n");
		printf("Exiting program\n");
		return 0;
	}

	FILE* matrixFile1 = NULL;
	FILE* matrixFile2 = NULL;
	FILE* outputFile = NULL;
	matrixFile1 = fopen(argv[1], "r");
	matrixFile2 = fopen(argv[2], "r");
	outputFile = fopen(argv[3], "w");

	if(matrixFile1 == NULL)
    	{
        	printf("\nERROR: %s was not found\n", argv[1]);
		printf("Exiting program\n");
        	return 0;
	}
	else if(matrixFile2 == NULL)
        {
                printf("\nERROR: %s was not found\n", argv[2]);
                printf("Exiting program\n");
                return 0;
        }
	
	int m1RowsNum, m1ColsNum, m2RowsNum, m2ColsNum;
	int secs = GetSecs(argv[4]);

	fscanf(matrixFile1, " %d", &m1RowsNum);
	fscanf(matrixFile1, " %d\n", &m1ColsNum);
	fscanf(matrixFile2, " %d", &m2RowsNum);
        fscanf(matrixFile2, " %d\n", &m2ColsNum);
	
	int* matrix1[m1RowsNum];
	for(int a = 0; a < m1RowsNum; a++)
		matrix1[a] = (int*) malloc(m1ColsNum * sizeof(int));

	int* matrix2[m2RowsNum];
        for(int a = 0; a < m2RowsNum; a++)
                matrix2[a] = (int*) malloc(m2ColsNum * sizeof(int));
	
	for(int i = 0; i < m1RowsNum; i++)
		for(int j = 0; j < m1ColsNum; j++)
			fscanf(matrixFile1, " %d", &matrix1[i][j]);
	
	 for(int i = 0; i < m2RowsNum; i++)
                for(int j = 0; j < m2ColsNum; j++)
                        fscanf(matrixFile2, " %d", &matrix2[i][j]);
	
	fclose(matrixFile1);
	fclose(matrixFile2);
	
	//InitTest(matrix1, matrix2, outputFile, m1RowsNum, m1ColsNum, m2RowsNum, m2ColsNum, secs);
	int NumThreads = m1RowsNum * m2ColsNum; //number of entries that will exist in the output matrix
	key_t key;
	int msgid;

	key = ftok("ttobrien", 1);
	msgid = msgget(key, IPC_CREAT | IPC_EXCL);
	if(msgid == -1)
		msgid = msgget(key, 0);

	PreMsg* outgoing;
        outgoing = (PreMsg*) malloc(NumThreads * sizeof(PreMsg));
	
	pthread_t threads[NumThreads];
	for(int i = 0; i < NumThreads; i++)
	{
		outgoing[i].typeP = 1;
		outgoing[i].jobidP = i;
		outgoing[i].rowvecP = i/m1RowsNum;
		outgoing[i].colvecP = i%m2ColsNum;
		outgoing[i].innerDimP = m1ColsNum;
		 
		for(int a = 0; a < m1ColsNum; a++)
		{
			outgoing[i].dataP[a] = matrix1[i/m1RowsNum][a];
			outgoing[i].dataP[m1ColsNum + a] = matrix2[a][i%m2ColsNum];
		}

		outgoing[i].mqidP = msgid;
   		pthread_create(&threads[i], NULL, ProducerSend, &outgoing[i]);
		sleep(secs);
	}
	
	fclose(outputFile);
	return 0;
}

void* ProducerSend(void* infoVoid)
{
	PreMsg* info = (PreMsg*)infoVoid;
	Msg* message;
        message	= (Msg*) malloc(sizeof(Msg));
	message->type = info->typeP;
	message->jobid = info->jobidP;
	message->rowvec = info->rowvecP;
	message->colvec = info->colvecP;
	message->innerDim = info->innerDimP;
	for(int a = 0; a < message->innerDim * 2; a++)
        {
        	message->data[a] = info->dataP[a];
        }

	int msgid = info->mqidP;
	pthread_mutex_lock(&lock1);
	int rc = msgsnd(msgid, &message, sizeof(message), 0); 
	NumJobsSent++;
	printf("Sending job id %d type %ld size %ld (rc=%d)\n", message->jobid, message->type, sizeof(message), rc);
	pthread_mutex_unlock(&lock1);
}


void InitTest(int** m1, int** m2, FILE* output, int rows1, int cols1, int rows2, int cols2, int secs)
{
	fprintf(output, "Matrix 1:\n");
	for(int i = 0; i < rows1; i++)
	{
                for(int j = 0; j < cols1; j++)
                        fprintf(output, "%d ", m1[i][j]);
		fprintf(output, "\n");
	}

	fprintf(output, "\nMatrix 2:\n");
	for(int i = 0; i < rows2; i++)
        {
                for(int j = 0; j < cols2; j++)
                        fprintf(output, "%d ", m2[i][j]);
                fprintf(output, "\n");
        }
	
	fprintf(output, "\nSecs: %d", secs);
	return;
}


int GetSecs(char* arg5)
{
	int i, secs;
	int len = strlen(arg5);
	for(i = 0; i < len; i++)
	{
		if(! isdigit(arg5[i]))
		{
			printf("\nERROR: integer expected for arguement 5\n");
                	printf("Exiting program\n");
			exit(1);
		}
	}
	sscanf(arg5, "%d", &secs);
	return secs;
}

