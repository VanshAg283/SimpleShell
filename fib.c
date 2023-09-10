#include<stdio.h>
#include<stdlib.h>

int fib(int n) {
  if(n<2) {return n;}
  else return fib(n-1)+fib(n-2);
}

int main(int argc, char **argv) {
	if(argc < 2) {
		printf("Please provide a number as an argument.\n");
		return 1;
	}
	int val = fib(atoi(argv[1]));
	printf("Value: %d \n",val);
	return 0;
}
