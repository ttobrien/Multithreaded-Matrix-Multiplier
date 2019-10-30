typedef stuct Computed{
	long type;
	int jobid;
	int rowvec;
	int colvec;
	int dotProduct;
} Entry;

void* DotProduct(void*);


int main(int argc, char *argv[])
{
}

void* DotProduct(void* param)
{
	//get job off queue
	//read in vars
	
	Entry* sendBack = (Entry*)malloc(sizeof(Entry));
	dp = 0;
	for(int i = 0; i < inner; i++)
	{
		dp = dp + nums[i] * nums[inner + i];
	}
	sendBack.dotProduct = dp;
}

