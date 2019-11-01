#include <stdio.h>
#include <signal.h>

// https://www.geeksforgeeks.org/write-a-c-program-that-doesnt-terminate-when-ctrlc-is-pressed/

int count = 0;

void ctrl_c_handler(int sig_num)
{
	signal(SIGINT, ctrl_c_handler);
	printf("\nCount = %d\n", count);
	fflush(stdout);
}



int main(void)
{
	signal(SIGINT, ctrl_c_handler);
	while(1)
		count++;
	return 0;
}
