all: package compute
.PHONY: all

package:
	 gcc -Wall -std=c99 -D_GNU_SOURCE -o package package.c -lpthread -g

compute: 
	 gcc -Wall -std=c99 -D_GNU_SOURCE -o compute compute.c -lpthread -g

clean:
	rm -r package compute
