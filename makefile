package:
	gcc -Wall -std=c99 package.c -o package -lpthread
compute:
	gcc -Wall -std=c99 compute.c -o compute -lpthread

