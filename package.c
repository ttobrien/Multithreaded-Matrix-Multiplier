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

/*pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;

typedef struct RoWCol{
	int row;
	int col;
} RowCol;
*/
int GetSecs(char*);
void InitTest(int**, int**, FILE*, int, int, int, int, int);

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

	InitTest(matrix1, matrix2, outputFile, m1RowsNum, m1ColsNum, m2RowsNum, m2ColsNum, secs);
	/*
	pthread_t threads[m1RowsNum];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	for(i=0; i<NUMTHRDS; i++)
   		pthread_create(&threads[i], &attr, dotprod, (void *)i);
	*/
	
	fclose(outputFile);
	return 0;
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

