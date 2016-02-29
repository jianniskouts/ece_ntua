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
#define SHELL_EXECUTABLE_NAME "shell" /* executable for shell */

/*Struct Process ID*/

struct PT{
	int myid;
	pid_t mypid;
	char name[TASK_NAME_SZ];
	struct PT *next;
};
typedef struct PT ProcessTable ;

/*id for every process*/
int id;
/*Head&Tail of queue*/
ProcessTable *head;
ProcessTable *tail;

void children(const char *name){
	pid_t mypid = getpid();
	printf("I am child %ld",(long)mypid);

	char executable[TASK_NAME_SZ];
	strcpy(executable,name);

	char *newargv[] = { executable, NULL, NULL, NULL};
	char *newenviron[] = { NULL };

	printf("About to replace myself with the executable %s... \n", executable);

	sleep(2);
	execve(executable,newargv,newenviron);

	printf("Execve failed...\n");
	perror("execve");
	exit(1);
		
}

void enqueue(ProcessTable node, int flag){
	if(head == NULL){
		head = malloc(sizeof(ProcessTable));
		head->mypid = node.mypid;
		if(flag == 1){
			head->myid = id;
			id++;
		}
		strcpy(head->name,node.name);
		head->next = NULL;
		tail = head;
	}
	else{
		ProcessTable *new = malloc(sizeof(ProcessTable));
		new->mypid = node.mypid;
		if(flag == 1){
			new->myid = id;
			id++;
		}
		else{
			new->myid = node.myid;
		}
		strcpy(new->name,node.name);
		new->next = NULL;
		tail->next = new;
		tail = new;
	}
}

ProcessTable dequeue(){
	ProcessTable *temp,dequeued;

	dequeued.mypid = head->mypid;
	dequeued.myid = head->myid;
	strcpy(dequeued.name,head->name);
	temp = head;
	head = head->next;
	free(temp);
	return dequeued;
}

int remove_dead_child(pid_t dead_process){
	ProcessTable *temp;
	ProcessTable *pretail;
//First scenario: 
	if(head == tail){
		free(head);
		return 1;
	}
//Second scenario:
	if(head->mypid == dead_process){
		temp = head;
		head = head->next;
		free(temp);
		return 2;
	}	
	temp = head;
	pretail = head;
//Third scenario:
	while(temp!= NULL){
		if(temp->mypid == dead_process){
			if(temp->next == NULL){
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

/*: Print a list of all tasks currently being scheduled.  */
static void
sched_print_tasks(void)
{	
	ProcessTable *temp;
	temp = head;


	while(temp != NULL){
		printf("name : %s | id =  %d | pid = %d \n", temp->name, temp->myid, temp->mypid);
		temp = temp -> next;
	}	
	//assert(0 && "Please fill me!");
}

/*: Send SIGKILL to a task determined by the value of its
 * scheduler-specific id.
 */
static int
sched_kill_task_by_id(int my_id)
{
	//assert(0 && "Please fill me!");
	ProcessTable *temp;
	temp = head;
	while(temp != NULL){
		if((temp->myid) == my_id){
			kill((temp->mypid),SIGTERM);
			break;
		}
	 	temp = temp -> next;
	}
	return -ENOSYS;
}


/* Create a new task.  */
static void
sched_create_task(char *executable)
{
	//assert(0 && "Please fill me!");//
	
	pid_t p = fork();
	if (p < 0){
		perror("fork");
		exit(1);
	}

	if (p == 0){
		raise(SIGSTOP);
		children(executable);
	}
	ProcessTable process;
	process.mypid = p;
	strcpy(process.name,executable);
	enqueue(process,1);
}

/* Process requests by the shell.  */
static int
process_request(struct request_struct *rq)
{
	switch (rq->request_no) {
		case REQ_PRINT_TASKS:
			sched_print_tasks();
			return 0;

		case REQ_KILL_TASK:
			return sched_kill_task_by_id(rq->task_arg);

		case REQ_EXEC_TASK:
			sched_create_task(rq->exec_task_arg);
			return 0;

		default:
			return -ENOSYS;
	}
}

/* 
 * SIGALRM handler
 */
static void
sigalrm_handler(int signum)
{
	//assert(0 && "Please fill me!");//
	if(signum!= SIGALRM){
		fprintf(stderr,"Internal error: Called for signum %d,not SIGALRM",signum);
		exit(1);
	
	}

	printf("ALARM! %d seconds have passes.\n",SCHED_TQ_SEC);

	ProcessTable remove_from_queue;

	remove_from_queue = dequeue();
	enqueue(remove_from_queue,0);

	kill(remove_from_queue.mypid,SIGSTOP);
	kill(head->mypid,SIGCONT);

	if(alarm(SCHED_TQ_SEC)<0){
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
	pid_t p;
	int status;

	if (signum!= SIGCHLD){
		fprintf(stderr,"Internal error: Called for signum %d, not SIGCHLD\n",signum);
		exit(1);
	 }
	for(;;){
	      	p = waitpid(-1, &status, WUNTRACED | WNOHANG);
		if (p<0) {
			perror("waitpid");
			exit(1);
		}
		
		if (p==0)  break;
		explain_wait_status(p,status);
		
	
		if (WIFEXITED(status) || WIFSIGNALED(status)){
			printf("Parent: Received SIGCHLD,child is dead.Removing...\n");
			int result = remove_dead_child(p);

			if(result == 1){
				printf("All tasks have been accomplished,exiting...\n");
				exit(1);
			}
				
			else if (result == 2){
				kill(head->mypid,SIGCONT);
				printf("Refreshing alarm for next child...\n");
				if (alarm(SCHED_TQ_SEC)<0){
					perror("alarm");
					exit(1);
					
				}
				
			}
				
			if (WIFSTOPPED(status)){
				printf("Parent: Child has been stopped. Moving right along..\n");
			}
		}
	}
}											                
/* Disable delivery of SIGALRM and SIGCHLD. */
static void
signals_disable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
		perror("signals_disable: sigprocmask");
		exit(1);
	}
}

/* Enable delivery of SIGALRM and SIGCHLD.  */
static void
signals_enable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
		perror("signals_enable: sigprocmask");
		exit(1);
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

static void
do_shell(char *executable, int wfd, int rfd)
{
	char arg1[10], arg2[10];
	char *newargv[] = { executable, NULL, NULL, NULL };
	char *newenviron[] = { NULL };

	sprintf(arg1, "%05d", wfd);
	sprintf(arg2, "%05d", rfd);
	newargv[1] = arg1;
	newargv[2] = arg2;

	raise(SIGSTOP);
	execve(executable, newargv, newenviron);

	/* execve() only returns on error */
	perror("scheduler: child: execve");
	exit(1);
}

/* Create a new shell task.
 *
 * The shell gets special treatment:
 * two pipes are created for communication and passed
 * as command-line arguments to the executable.
 */
static void
sched_create_shell(char *executable, int *request_fd, int *return_fd)
{
	pid_t p;
	int pfds_rq[2], pfds_ret[2];

	if (pipe(pfds_rq) < 0 || pipe(pfds_ret) < 0) {
		perror("pipe");
		exit(1);
	}

	p = fork();
	if (p < 0) {
		perror("scheduler: fork");
		exit(1);
	}

	if (p == 0) {
		/* Child */
		close(pfds_rq[0]);
		close(pfds_ret[1]);
		do_shell(executable, pfds_rq[1], pfds_ret[0]);
		assert(0);
	}
	/* Parent p = p_shell store it in queue */
	close(pfds_rq[1]);
	close(pfds_ret[0]);
	*request_fd = pfds_rq[0];
	*return_fd = pfds_ret[1];
	
	//Store in queue
	ProcessTable process;
	process.mypid = p;
	strcpy(process.name, SHELL_EXECUTABLE_NAME);
	enqueue(process,1);
		
}

static void
shell_request_loop(int request_fd, int return_fd)
{
	int ret;
	struct request_struct rq;

	/*
	 * Keep receiving requests from the shell.
	 */
	for (;;) {
		if (read(request_fd, &rq, sizeof(rq)) != sizeof(rq)) {
			perror("scheduler: read from shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}

		signals_disable();
		ret = process_request(&rq);
		signals_enable();

		if (write(return_fd, &ret, sizeof(ret)) != sizeof(ret)) {
			perror("scheduler: write to shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int i,nproc;
	id = 0;
	pid_t p;
	tail = NULL;
	head = NULL;

	/* Two file descriptors for communication with the shell */
	static int request_fd, return_fd;

	/* Create the shell. */
	sched_create_shell(SHELL_EXECUTABLE_NAME, &request_fd, &return_fd);
	/*  add the shell to the scheduler's tasks->mallon egine */

	/*
	 * For each of argv[1] to argv[argc - 1],
	 * create a new child process, add it to the process list.
	 */
	

	nproc = argc-1; /* number of proccesses goes here */
	
	for(i=0; i<nproc; i++){
		p = fork();
		if(p < 0){
			perror("fork");
			exit(1);
		}
		else if(p == 0){
			//Child
			raise(SIGSTOP);
			children(argv[i+1]);
		}
		else {
			printf("Father process enqueue..\n");
			ProcessTable process;
			process.mypid = p;
			strcpy(process.name,argv[i+1]);
			enqueue(process,1);
		}	
	}

	/* Wait for all children to raise SIGSTOP before exec()ing. */
	wait_for_ready_children(nproc);

	/* Install SIGALRM and SIGCHLD handlers. */
	install_signal_handlers();

	if (nproc == 0) {
		fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
		exit(1);
	}
	
	printf("All children are ready,start schedule....\n");
	kill(head->mypid,SIGCONT);
	alarm(SCHED_TQ_SEC);
	
	shell_request_loop(request_fd, return_fd);

	/* Now that the shell is gone, just loop forever
	 * until we exit from inside a signal handler.
	 */
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}
