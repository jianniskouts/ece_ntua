#include<stdio.h>
#include<unistd.h>


void zing(){

	char* i;
	i  = getlogin();
	printf("%s is awesome\n",i);
}
