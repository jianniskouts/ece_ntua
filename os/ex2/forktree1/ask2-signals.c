#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"

#include "tree.h"

#define SLEEP_TREE_SEC 3


void fork_procs(struct tree_node *node );

pid_t do_fork(struct tree_node *root){
	//int n = 0;
	pid_t p;
	printf("%s\n",root->name );
	p = fork();
	if(p < 0){
		perror("fork");
		exit(1);
	}
	if(p == 0){
		fork_procs(root);
		
	}
	return p;

}	
void fork_procs(struct tree_node *node )
{

	int status;
	pid_t pid[100],pid_temp;
	int i;
	change_pname(node->name);
	printf("PID = %ld, name %s, starting...\n",
                        (long)getpid(), node->name);
  	if(node->nr_children > 0){
		for(i = 0; i<node->nr_children; i++){
			pid[i] = do_fork(node->children+i);
		}
		wait_for_ready_children(node->nr_children);
		
		raise(SIGSTOP);
		printf("PID = %ld, name = %s is awake\n",
                (long)getpid(), node->name);
		for(i = 0; i<node->nr_children; i++){
			kill(pid[i],SIGCONT);
		}
		for(i = 0; i<node->nr_children;i++){
			pid_temp = wait(&status);
			explain_wait_status(pid_temp,status);
		}
		exit(12);
	}
	else if(&node->nr_children < 0){
		perror("wrong ipnut");
		exit(1);
	
	}
	else{
		raise(SIGSTOP);
		printf("PID = %ld, name = %s is awake\n",
                (long)getpid(), node->name);
		exit(13);
	}
}

/*
 * The initial process forks the root of the process tree,
 * waits for the process tree to be completely created,
 * then takes a photo of it using show_pstree().
 *
 * How to wait for the process tree to be ready?
 * In ask2-{fork, tree}:
 *      wait for a few seconds, hope for the best.
 * In ask2-signals:
 *      use wait_for_ready_children() to wait until
 *      the first process raises SIGSTOP.
 */
int main(int argc, char *argv[])
{
	if(argc != 2){
		fprintf(stderr, "Usage %s <input_tree_file>\n\n",argv[0]);
		exit(1);
	}
	pid_t pid_root;
	struct tree_node *root;
	int status;
	root = get_tree_from_file(argv[1]);
	/* Fork root of process tree */
	pid_root = fork();
	if (pid_root < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid_root == 0) {
		/* Child */
		fork_procs(root);
		exit(1);
	}

	/*
	 * Father
	 */
	/* for ask2-signals */
	wait_for_ready_children(1); 

	/* Print the process tree root at pid */
	show_pstree(pid_root);

	/* for ask2-signals */
	kill(pid_root, SIGCONT);

	/* Wait for the root of the process tree to terminate */
	pid_root = wait(&status);
	explain_wait_status(pid_root, status);

	return 0;
}
