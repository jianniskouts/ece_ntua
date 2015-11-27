#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"
#include "tree.h"


void fork_procs(struct tree_node *node, int pfdw[]);

pid_t do_fork(struct tree_node *root, int pfd[]){
	pid_t p;
	p = fork();
	if(p < 0){
		perror("fork");
		exit(1);
	}
	if(p == 0){
		fork_procs(root,pfd);
	}
	return p;

}	
void fork_procs(struct tree_node *node, int pfdw[]){
	int status;
	pid_t pid[100],pid_temp;
	int pfd[2];
	int temp;
	int i;
	int val[2];
	change_pname(node->name);
	printf("PID = %ld, name %s, starting...\n", (long)getpid(), node->name);
  	if(node->nr_children > 0){
  		if (pipe(pfd) < 0) {
			perror("pipe");
			exit(1);
		}
		printf("Parent: Creating pipe...\n");
		for(i = 0; i < 2; i++){
			pid[i] = do_fork(node->children+i,pfd);
		}
		wait_for_ready_children(node->nr_children);
		raise(SIGSTOP);
		printf("PID = %ld, name = %s is awake\n",
                (long)getpid(), node->name);
		for(i = 0; i < 2; i++){
			kill(pid[i],SIGCONT);
		}
		for(i = 0; i<node->nr_children;i++){
			pid_temp = wait(&status);
			explain_wait_status(pid_temp,status);
		}
		for (i = 0; i < 2; ++i){
			if (read(pfd[0], &val[i], sizeof(val[i])) != sizeof(val[i])) {
				perror("read from pipe");
				exit(1);
			}
		}
		if (strcmp(node->name,"+") == 0){
			temp = val[0] + val[1];
		}
		else if(strcmp(node->name,"*") == 0){
			temp = val[0] * val[1];
		}
	
		if (write(pfdw[1], &temp, sizeof(temp)) != sizeof(temp)) {
			perror("parent: write to pipe");
			exit(1);
		}
		exit(12);
	}
	else if(&node->nr_children < 0){
		perror("wrong input");
		exit(1);
	
	}
	else{
		temp = atoi(node->name);
		raise(SIGSTOP);
		printf("PID = %ld, name = %s is awake\n", (long)getpid(), node->name);
		if (write(pfdw[1], &temp, sizeof(temp)) != sizeof(temp)) {
			perror("write to pipe");
			exit(1);
		}
		exit(13);
	}
}

/*
 * The initial process forks the root of the process tree,
 * waits for the process tree to be completely created,
 * then takes a photo of it using show_pstree().
 *
 * 
 * We wait for the process to be ready by 
 * using wait_for_ready_children() to wait 
 * until the first process raises SIGSTOP. 
 * After the SIGCONT from father process 
 * to child process, the child writes its value
 * the father, where the result of the branch is 
 * computed.
 */
int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr, "Usage %s <input_tree_file>\n\n",argv[0]);
		exit(1);
	}
	int result;
	int pfd_init[2];
	pid_t pid_root;
	struct tree_node *root;
	int status;
	root = get_tree_from_file(argv[1]);
	/*if ((strcmp(root->name,"+") != 0) && (strcmp(root->name,"+") != 0)){
			printf("wrong input file, try again. \n");
			exit(1);

	}*/
	printf("Parent: Creating pipe...\n");
	if (pipe(pfd_init) < 0) {
		perror("pipe");
		exit(1);
	}
	/* Fork root of process tree */
	pid_root = fork();
	if (pid_root < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid_root == 0) {
		/* Child */
		fork_procs(root,pfd_init);
		exit(1);
	}
	/*
	 * Father
	 */
	wait_for_ready_children(1); 
	/* Print the process tree root at pid */
	show_pstree(pid_root);
	kill(pid_root, SIGCONT);
	/* Wait for the root of the process tree to terminate */
	pid_root = wait(&status);

	if (read(pfd_init[0], &result, sizeof(result)) != sizeof(result)) {
		perror("read from pipe");
		exit(1);
	}

	explain_wait_status(pid_root, status);

	/*Print the result of the file*/
	printf("%d\n",result );

	return 0;
}
