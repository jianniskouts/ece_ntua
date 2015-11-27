#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

/*
 * Create this process tree:
 * A-+-B---D
 *   `-C
 */
void fork_procs(void)
{
	/*
	 * initial process is A.
	 */
	int status;
	change_pname("A");
	printf("A: Sleeping...\n");
  	pid_t pid_B;
	pid_B = fork();
	if(pid_B < 0){
		perror("fork_B");
		exit(1);
	}
	if(pid_B == 0){
		pid_t pid_D;
		change_pname("B");
		printf("B: Sleeping...\n");
		pid_D = fork();	
		if(pid_D < 0){
			perror("fork_D");
			exit(1);
		}
		if(pid_D == 0){
			change_pname("D");
			printf("D: Sleeping...\n");
			sleep(SLEEP_PROC_SEC);
			printf("D: Exiting...\n");
			exit(13);
		}
		sleep(SLEEP_PROC_SEC);
		pid_D = wait(&status);
		explain_wait_status(pid_D,status);
		printf("B: Exiting...\n");
		exit(19);
	}
	
	pid_t pid_C;
	pid_C = fork();
	if(pid_C < 0){
		perror("fork_C");
		exit(1);
	}
	if(pid_C == 0){
		change_pname("C");
		printf("C: Sleeping...\n");
		sleep(SLEEP_PROC_SEC);
		printf("C: Exiting...\n");
		exit(17);
	}
	sleep(SLEEP_TREE_SEC);
	pid_B = wait(&status);
	explain_wait_status(pid_B,status);
	pid_B = wait(&status);
	explain_wait_status(pid_B,status);

	printf("A: Exiting...\n");
	exit(16);
}

/*
 * The initial process forks the root of the process tree,
 * waits for the process tree to be completely created,
 * then takes a photo of it using show_pstree().
 *
 * We wait for a few seconds 
 * in order for the process tree to be ready, 
 * hope for the best.
 */
int main(void)
{
	pid_t pid;
	int status;

	/* Fork root of process tree */
	pid = fork();
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		/* Child */
		fork_procs();
		exit(1);
	}

	/*
	 * Father
	 */

	sleep(SLEEP_TREE_SEC);

	/* Print the process tree root at pid */
	show_pstree(pid);

	/* Wait for the root of the process tree to terminate */
	pid = wait(&status);
	explain_wait_status(pid, status);

	return 0;
}
