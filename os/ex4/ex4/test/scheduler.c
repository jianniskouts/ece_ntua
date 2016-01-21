#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"
#include "request.h"

/* Compile-time parameters. */
#define SCHED_TQ_SEC 2                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */


struct child{
	pid_t mypid;
	struct child *next;
};
typedef struct child child;

child *head;
child *tail;
	

void children(const char *name){
	pid_t mypid = getpid();
	printf("I am child %ld",(long)mypid);
	
	char executable[TASK_NAME_SZ];
	strcpy(executable,name);

	char *newargv[] = { executable, NULL, NULL, NULL };
	char *newenviron[] = { NULL };
	
	printf("About to replace myself with the executable %s....\n",executable);
	sleep(2);

	execve(executable, newargv, newenviron);
	
	printf("Execve failed,exiting...\n");
	perror("execve");
	exit(1);

}



void enqueue(pid_t pid){
	
	//Queue is empty
	if(head == NULL){
		head = malloc(sizeof(child));
		head->mypid = pid;
		head->next = NULL;
		tail = head;
	}
	else {
		child *new = malloc(sizeof(child));
		new->mypid = pid;
		new->next = NULL;
		tail->next = new;
		tail = new;

	}
}	
pid_t dequeue(){
	child *temp;
	pid_t head_pid;
	head_pid = head->mypid;
	temp = head;
 	head = head->next;
	free(temp);
	return head_pid;
}	

int remove_dead_child(pid_t dead_child){
/*
 *3 possible scenarios :
 *1)The child died and queue is empty => All tasks have been accomplished.
 *2)The child, first in the queue, died => Remove it from queue and renew timer on the next one.
 *3)A child between head and tail died => Simply remove it from queue.
 */

	child *temp;
	child *pretail;
//First scenario :
	if(head == tail){
		free(head);
		return 1;
	}	
//Second scenario :
	if(head->mypid == dead_child){
		temp = head;
		head = head->next;
		free(temp);
		return 2;
	}
	temp = head;
	pretail = head;
//Third scenario :
	while (temp != NULL){
       
	if (temp->mypid == dead_child){
            	if (temp->next == NULL){
                	tail = pretail;
			tail->next = NULL;
               		free(temp);
                	return 0;
            	}
            	else{
                	pretail->next = temp->next;
                	free(temp);
                	return 0;
            	}
        }
        else{
            pretail = temp;
            temp = temp->next;
        }
    }
	return 0;
}

/*
 * SIGALRM handler
 */
static void
sigalrm_handler(int signum)
{
	//assert(0 && "Please fill me!");//
	if (signum != SIGALRM ) {
		fprintf(stderr, " Internal error: Called for signum %d,not SIGALRM\n",signum);
		exit(1);
	}
	
	printf("ALARM! %d seconds have passed.\n", SCHED_TQ_SEC);
		
	pid_t remove_from_queue;

	remove_from_queue = dequeue();
	enqueue(remove_from_queue);

	kill(remove_from_queue,SIGSTOP);
	kill(head->mypid,SIGCONT);

	if (alarm(SCHED_TQ_SEC)<0){
		perror("alarm");
		exit(1);
	}
	
	
}

/* 
 * SIGCHLD handler
 */
static void
sigchld_handler(int signum)
{
	
	//assert(0 && "Please fill me!");//
	pid_t p;
	int status;

	if (signum!= SIGCHLD){
		fprintf(stderr,"Internal error: Called for signum %d, not SIGCHLD\n",signum);
		exit(1);

	}	

	for(;;) {
		p = waitpid(-1, &status, WUNTRACED | WNOHANG);
		if (p<0) {
			perror("waitpid");
			exit(1);	
	
		}
		if (p==0)  break;
		
		explain_wait_status(p,status);

		if(WIFEXITED(status) || WIFSIGNALED(status)){
			printf("Parent: Received SIGCHLD,child is dead. Removing..\n");
		        int result = remove_dead_child(p);  
			
			if(result == 1){
			 	printf("All tasks have been accomplished,exiting ....\n");
				exit(1);
			}
			else if(result == 2){
				kill(head->mypid,SIGCONT);
				printf("Refreshing alarm for next child..\n");
				
  				if(alarm(SCHED_TQ_SEC)<0){
                			 perror("alarm");
               				 exit(1);
        			}
			}
		
			if(WIFSTOPPED(status)){
				printf("Parent: Child has been stopped. Moving right along..\n");
			}
	 	}
	}
}

/* Install two signal handlers.
 * One for SIGCHLD, one for SIGALRM.
 * Make sure both signals are masked when one of them is running.
 */
static void
install_signal_handlers(void)
{
	sigset_t sigset;
	struct sigaction sa;

	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGCHLD);
	sigaddset(&sigset, SIGALRM);
	sa.sa_mask = sigset;
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		perror("sigaction: sigchld");
		exit(1);
	}

	sa.sa_handler = sigalrm_handler;
	if (sigaction(SIGALRM, &sa, NULL) < 0) {
		perror("sigaction: sigalrm");
		exit(1);
	}

	/*
	 * Ignore SIGPIPE, so that write()s to pipes
	 * with no reader do not result in us being killed,
	 * and write() returns EPIPE instead.
	 */
	if (signal(SIGPIPE, SIG_IGN) < 0) {
		perror("signal: sigpipe");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	int i,nproc;
	pid_t p;
	tail = NULL;
	head = NULL;
	/*
	 * For each of argv[1] to argv[argc - 1],
	 * create a new child process, add it to the process list.
	 */
	
       	nproc = argc - 1; /* number of proccesses goes here */

	/* Wait for all children to raise SIGSTOP before exec()ing. */
	//wait_for_ready_children(nproc);//

	/* Install SIGALRM and SIGCHLD handlers. */
	
	 if (nproc == 0) {
                fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
                exit(1);
        }

	for(i=0; i<nproc; i++){
		p = fork();
		if(p<0){
			perror("fork");
			exit(1);
		}
		else if (p == 0){
		/*
		 *Child
		 */
			printf("Child created with pid = %ld \n" ,(long)getpid());
			printf("Will now stop until all children will be created...\n");
			raise(SIGSTOP);
			children(argv[i+1]);
		}
		else {
		/*
		 *Father
		 */
			printf("Father process enqueue \n");
			enqueue(p);		
		}
	}	

	wait_for_ready_children(nproc);	
	install_signal_handlers();

	printf("All children have been created...\n");
	printf("Scheduler starting...\n");
	kill(head->mypid,SIGCONT);
	
	alarm(SCHED_TQ_SEC);

	/* loop forever  until we exit from inside a signal handler. */
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}
