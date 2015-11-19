#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>


void doWrite(int fd, const char* buff, int len){
	ssize_t wr;
	wr = write(fd, buff, len);//write to fd from buff
	if (wr < 0) {
      		perror("write");
      		exit(1);
	} else if (wr == 0) {
      		printf("write returned zero?!\n");
	} else{
      		printf("I wrote %d bytes\n", wr);
	}
}

void write_file(int fd, const char* infile){
		int fdw;
		fdw = open(infile,O_RDONLY);//open the source file to read from
		
		if(fdw == -1){
			perror("open");
			exit(1);
		}
		int j;
		char buf[1024];
		for(j = 0;j < 1024; j++){
			buf[j] = 0;
		}
		ssize_t ret;
		ret = read(fdw, buf, 1024);//read from argv[1] and argv[2]
		if (ret < 0) {
  	    		perror("read");
      			exit(1);
		} else if (ret == 0) {
			printf("I am at end-of-file\n");
		//} else if (ret >= 999){
		//	printf("The file is too big for the process\n ");
		//	exit(1);
		}else {
      			printf("I read %d bytes\n", ret);
		}
		if (close(fdw) < 0){
			perror("close");
			exit(1);
		}	
		doWrite(fd, buf,strlen(buf)); //call the function to write on fd from buf
}


int main(int argc, char ** argv){
	if (argc < 3){
		printf("Usage: ./fconc infile1 infile2 [outfile (default:fconc.out)]\n");
		exit(1);
	}
	char* file;
	
	int fd1, oflags, mode;
	oflags = O_CREAT | O_WRONLY | O_TRUNC; //set flags for open syscall
	mode = S_IRUSR | S_IWUSR; //set mode for open syscall

	fd1 = open("C", oflags, mode);//open the file I want to write on
	if (fd1 == -1){
		perror("open");
		exit(1);
	}
	int i = 0;
	for(i = 1; i < 3; i++){
		write_file(fd1,argv[i]); //I call the functions of writing to the previous file
	}

	return 0;
}
	
