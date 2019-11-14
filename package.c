//Author: Tommy O'Brien
//Start date: 10-26-2019

#include "header.h"

void* ProducerSend(void*);
int GetSecs(char*);
void InitTest(int**, int**, FILE*, int, int, int, int, int);

int NumJobsSent = 0;
int NumJobsRec = 0;

//Source: https://www.geeksforgeeks.org/write-a-c-program-that-doesnt-terminate-when-ctrlc-is-pressed/
void ctrl_c_handler(int sig_num)
{
        signal(SIGINT, ctrl_c_handler);
        printf("^CJobs sent %d Jobs Recieved %d\n", NumJobsSent, NumJobsRec);
        fflush(stdout);
}


int main(int argc, char *argv[])
{
	signal(SIGINT, ctrl_c_handler);

	assert(argc == 5);

	FILE* matrixFile1 = NULL;
	FILE* matrixFile2 = NULL;
	FILE* outputFile = NULL;
	matrixFile1 = fopen(argv[1], "r");
	matrixFile2 = fopen(argv[2], "r");
	outputFile = fopen(argv[3], "w");

	assert(matrixFile1 != NULL);
	assert(matrixFile2 != NULL);
              
	int m1RowsNum = 0, m1ColsNum = 0, m2RowsNum = 0, m2ColsNum = 0;
	int secs = GetSecs(argv[4]);

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
		matrix1[a] = (int*) malloc(m1ColsNum * sizeof(int));

	int** matrix2 = (int **) malloc(m2RowsNum * sizeof(int*));
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
	

	int** mOut = (int **) malloc(m1RowsNum * sizeof(int*));
        for(int a = 0; a < m1RowsNum; a++)
                mOut[a] = (int*) malloc(m2ColsNum * sizeof(int));
	//InitTest(matrix1, matrix2, outputFile, m1RowsNum, m1ColsNum, m2RowsNum, m2ColsNum, secs);
	int NumThreads = m1RowsNum * m2ColsNum; //number of entries that will exist in the output matrix
	msgctl(491520, IPC_RMID, NULL);	
	int msgid;
	key_t key = ftok("ttobrien", 11);
	
	msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
	if(msgid == -1)
		msgid = msgget(key, 0);

	
	

/*	PreMsg** outgoing = (PreMsg**)malloc(NumThreads* sizeof(PreMsg*)); //stackoverflow how to implement a 2-dimensional array of struct in C
	for(int h = 0; h <NumThreads; h++)
        	outgoing[h] = (PreMsg*) malloc(sizeof(PreMsg));
*/
	PreMsg* outgoing = (PreMsg*) malloc(NumThreads * sizeof(PreMsg));//stack over flow 10468128 how do you make an array of structs in C?
/*	for(int x = 0; x < NumThreads; x++)
	{
		outgoing[x].dataP* = (int*) malloc(100 * sizeof(int));
	}
*/		
	pthread_t threads[NumThreads];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	for(int i = 0; i < NumThreads; i++)
	{
		//outgoing[i].dataP = (int *) malloc(sizeof(int) * 100);
//		outgoing[i].typeP = 1;
//		outgoing[i].jobidP = i;
//		outgoing[i].rowvecP = i/m2ColsNum;
	//	printf("row: %d\n", outgoing[i].rowvecP);
//		outgoing[i].colvecP = i%m2ColsNum;
//		outgoing[i].innerDimP = m1ColsNum;
		 
//		for(int a = 0; a < m1ColsNum; a++)
//		{
//			outgoing[i].dataP[a] = matrix1[i/m2ColsNum][a];
//			outgoing[i].dataP[m1ColsNum + a] = matrix2[a][i%m2ColsNum];
//		}
//		for (int b = 2*m1ColsNum; b < 100; b++)
//			outgoing[i].dataP[b] = 0;

//		outgoing[i].mqidP = msgid;
   		outgoing[i].jobidP = i;
		outgoing[i].mqidP = &msgid;
		outgoing[i].m2C = &m2ColsNum;
		outgoing[i].m1C = &m1ColsNum;
		outgoing[i].m1 = matrix1;
		outgoing[i].m2 = matrix2;
		
		pthread_create(&threads[i], &attr, ProducerSend, &outgoing[i]);
		sleep(secs);
	}
	/*free(outgoing);
	for(int a = 0; a < m1RowsNum; a++)
		free(matrix1[a]);
	free(matrix1);
	for(int a = 0; a < m2RowsNum; a++)
		free(matrix2[a]);
	free(matrix2);*/
	//SHOULD FREE THIS AFTER ALL SENT BUT BEFORE ALL RECIEVED
	
	int rcJoin;
	ReturnEntry* e;
	void* ptr;

	for(int j = 0; j < NumThreads; j++)
	{
		rcJoin = pthread_join(threads[j], &ptr);
		assert(rcJoin == 0);
		e = (ReturnEntry*)ptr;
		mOut[e->row][e->col] = e->dp;
		free(e);
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
		free(mOut[a]);
	free(mOut);
	printf("\n\n\n");
	fclose(outputFile);
	
	free(outgoing);
        for(int a = 0; a < m1RowsNum; a++)
                free(matrix1[a]);
        free(matrix1);
        for(int a = 0; a < m2RowsNum; a++)
                free(matrix2[a]);
        free(matrix2);

	return 0;
}

void* ProducerSend(void* infoVoid)
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
	
	//printf("msgid: %d\n", msgid);
	pthread_mutex_lock(&lock1);
	int rc1 = msgsnd(msgid, &message, (4 + 2 * message.innerDim) * sizeof(int), 0);
      // if(rc == -1)
      // printf("\nUH OH: %s\n\n", strerror(errno));	       
	NumJobsSent++;
	printf("Sending job id %d type %ld size %ld (rc=%d)\n", message.jobid, message.type, (4 + 2 * message.innerDim) * sizeof(int), rc1);
	pthread_mutex_unlock(&lock1);
	//free(infoVoid);


	Entry entry;
	pthread_mutex_lock(&lock2);
	int rc2 = msgrcv(msgid, &entry, 4 * sizeof(int), 2, 0);
        assert(rc2 >= 0);
	printf("Recieving job id %d type %ld size %ld\n", entry.jobid, entry.type, 4 * sizeof(int));
	NumJobsRec++;
	pthread_mutex_unlock(&lock2);

	ReturnEntry* returnEntry;
	returnEntry = (ReturnEntry*) malloc(sizeof(ReturnEntry));
	returnEntry->row = entry.rowvec;
	returnEntry->col = entry.colvec;
	returnEntry->dp = entry.dotProduct;
//	printf("dp: %d\n", returnEntry->dp);
	return returnEntry;
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
		assert(isdigit(arg5[i]));
	
	sscanf(arg5, "%d", &secs);

	return secs;
}

