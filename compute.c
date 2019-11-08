#include "header.h"

void* DotProduct(void*);


int main(int argc, char *argv[])
{
	assert((argc == 2) || (argc == 3));

	int msgid;
        int key = 11829579;

        msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
        if(msgid == -1)
                msgid = msgget(key, 0);

	
/*	Msg message;
	//message = (Msg*) malloc(sizeof(Msg));
        msgrcv(msgid, &message, 416, 1, 0);
	printf("%d\n", message.jobid);
*/	
	
	pthread_t thread;
	pthread_create(&thread, NULL, DotProduct, &msgid);
	pthread_join(thread, NULL);
	
	
	
	
	
	
	
	return 0;
}

void* DotProduct(void* param)
{
	int id = 0, row = 0, col = 0, inner = 0;
	int  n = 0;
	int rc1 = 0, rc2 = 0;
	int msgid = *((int*)param);

	//get job off queue
	//read in vars
	Msg message;
        rc1 = msgrcv(msgid, &message, 104 * sizeof(int), 1, 0);
        assert(rc1 >= 0);
	//printf("%d\n", message.jobid);

	id = message.jobid;
	row = message.rowvec;
	col = message.colvec;
	inner = message.innerDim;

	printf("Recieving job id %d type %ld size %ld\n", id, message.type, (4 + 2 * inner) * sizeof(int));
	
	Entry sendBack;
	//sendBack = (Entry*)malloc(sizeof(Entry));
	int dp = 0;
	
	for(int i = 0; i < inner; i++)
	{
		dp = dp + message.data[i] * message.data[inner + i];
	}
	sendBack.dotProduct = dp;
	
	sendBack.type = 2;
	sendBack.jobid = id;
	sendBack.rowvec = row;
	sendBack.colvec = col;
	
	rc2 = msgsnd(msgid, &sendBack, 4 * sizeof(int) + sizeof(long), 0);
	if (n == 1)
	{
		printf("Sum for cell %d,%d is %d\n", row, col, dp);
	}
	else
	{
		printf("Sending job id %d type %ld size %ld (rc=%d)\n", id, sendBack.type, sizeof(long) + 4 * sizeof(int), rc2);
	}

	return 0;
}
