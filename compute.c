#include "header.h"

void* DotProduct(void*);


int main(int argc, char *argv[])
{
	assert((argc == 2) || (argc == 3));

}

void* DotProduct(void* param)
{
	int id = 0, row = 0, col = 0;
	int poolSize = 0, n = 0;
	int rc = 0;
	
	//get job off queue
	//read in vars
	
	
	//printf("Recieving job id %d type %d size %d");
	
	
	Entry* sendBack;
	sendBack = (Entry*)malloc(sizeof(Entry));
	int dp = 0;
	
	for(int i = 0; i < inner; i++)
	{
		dp = dp + nums[i] * nums[inner + i];
	}
	sendBack->dotProduct = dp;
	
	sendBack->type = 2;
	sendBack->jobid = id;
	sendBack->rowvec = row;
	sendBack->colvec = col;
	
	if (n == 1)
	{
		printf("Sum for cell %d,%d is %d\n", row, col, dp);
	}

	if (n == 0)
	{
		printf("Sending job id %d type 2 size %ld (rc=%d)", id, sizeof(sendBack), rc);
	}
}
