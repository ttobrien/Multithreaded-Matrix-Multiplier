#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	FILE* out;
	out = fopen("exBIG.dat", "w");
	fprintf(out, "50 50\n");
	for(int i = 0; i < 50*50; i++)
		fprintf(out, "%d ", i%10);
	fclose(out);
	return 0;
}
