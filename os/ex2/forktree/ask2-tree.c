#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"

#include "tree.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

/*
 * Create this process tree:
 * A-+-B---D
 *   `-C
 */
void fork_procs(struct tree_node *node );

static void do_fork(struct tree_node *root){
	pid_t pid;
	pid = fork();
	if(pid < 0){
		perror("fork");
		exit(1);
	}
	if(pid == 0){
		fork_procs(root);
	}

}	
void fork_procs(struct tree_node *node )
{

	int status;
	pid_t pid;
	int i = 0;
	change_pname(node->name);
	printf("%s: Sleeping...\n",node->name);
  	if(node->nr_children > 0){
		for(i = 0; i<node->nr_children; i++){
			do_fork(node->children+i);
		}
		sleep(SLEEP_PROC_SEC);
		for(i=0;i<node->nr_children;i++){
			pid = wait(&status);
			explain_wait_status(pid,status);	
		}
		printf("%s: Exiting...\n",node->name);
		exit(12);
	}
	else if(&node->nr_children < 0){
		perror("wrong ipnut");
		exit(1);
	
	}
	else{
		sleep(SLEEP_PROC_SEC);
		printf("%s: Exiting...\n",node->name);
		exit(13);
	}
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
int main(int argc, char *argv[])
{
	if(argc != 2){
		fprintf(stderr, "Usage %s <input_tree_file>\n\n",argv[0]);
		exit(1);
	}
	pid_t pid;
	struct tree_node *root;
	int status;
	root = get_tree_from_file(argv[1]);
	/* Fork root of process tree */
	pid = fork();
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		/* Child */
		fork_procs(root);
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
