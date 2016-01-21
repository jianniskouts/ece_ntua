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
#include "queue.h"

/* Compile-time parameters. */
#define SCHED_TQ_SEC 2                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */
#define SHELL_EXECUTABLE_NAME "shell" /* executable for shell */

/*the process queue*/
queue * q;

int id = 0;
/* Print a list of all tasks currently being scheduled.  */
static void
sched_print_tasks(void){
	node * temp ;
	temp = q->head;
	while(temp != NULL){
		printf("Tasks:\nid: %d | pid: %d,| name: %s\n",temp->p->myid,temp->p->pid,temp->p->name);
		temp = temp->pre;

	}
}

/* Send SIGKILL to a task determined by the value of its
 * scheduler-specific id.
 */
static int
sched_kill_task_by_id(int id){
	node *temp;
	temp = q->head;
	while(temp !=NULL){
		if (temp->p->myid == id){
			kill(temp->p->pid,SIGTERM); //why not SIGTERM??
			break;
		}
		temp = temp->pre;
	}
	return -ENOSYS;
}


/* Create a new task.  */
static void
sched_create_task(char *executable){
	struct process *proc;
	pid_t p = fork();
		if (p < 0) {
			perror("fork");
			exit(1);
		}

		if (p == 0) {
			char *newargv[] = { executable, NULL, NULL, NULL };
        		char *newenviron[] = { NULL };
			raise(SIGSTOP);
			execve(executable,newargv,newenviron);
			exit(1);
		}
		proc = malloc(sizeof(struct process));
		proc->pid = p;
		id++;
		proc->myid = id;
		strcpy(proc->name,executable);
		enqueue(proc,q);
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


/*
 * SIGALRM handler
 */
static void
sigalrm_handler(int signum){
	struct process * proc;
	proc = get_top(q);
	kill(proc->pid,SIGSTOP);
	
	 if (alarm(SCHED_TQ_SEC) < 0) {
                perror("alarm");
                exit(1);
        }
}

/* 
 * SIGCHLD handler
 */
static void
sigchld_handler(int signum){
	if (signum != SIGCHLD) {
                fprintf(stderr, "Internal error: Called for signum %d, not SIGCHLD\n",
                        signum);
                exit(1);
        }
	
	struct process *p;
	pid_t pid;
	int status;
	for (;;) {
            pid = waitpid(-1, &status, WUNTRACED | WNOHANG);
            if (pid < 0) {
                    perror("waitpid");
                    exit(1);
            }
            if (pid == 0)
                    break;

            explain_wait_status(pid, status);

            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    /* A child has died */
                   printf("Parent: Received SIGCHLD, child  %s is dead.\n", name_by_pid(pid,q));
				p = get_top(q);
				dequeue(q);
				p = get_top(q);

				if (p == NULL ) {
					printf("No more process\n");
					exit(1);
					}

				printf("starting process %s\n",p->name);
				kill(p->pid,SIGCONT);
					if (alarm(SCHED_TQ_SEC) < 0) { // reset timer
					perror("alarm");
					exit(1);
				}
			}
		if (WIFSTOPPED(status)) {
			/* A child has stopped due to SIGSTOP/SIGTSTP, etc... */
			printf("Parent: Child has been stopped. Moving right along...\n");
			p = dequeue(q);

			if (p == NULL ) {
				printf("No more process\n");
				exit(1);
				
			}
			enqueue(p,q);
			p = get_top(q);
			if (p == NULL) break;
			printf("starting process %s\n",p->name);
			kill(p->pid,SIGCONT);	
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
	struct process *proc;
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
	/* Parent */
	close(pfds_rq[1]);
	close(pfds_ret[0]);
	*request_fd = pfds_rq[0];
	*return_fd = pfds_ret[1];

	//Store in node list
	proc = malloc(sizeof(struct process));
	proc->pid = p;
	id++;
	proc->myid = id;//id for the shell?
	proc->name = executable;
	enqueue(proc,q);
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

int main(int argc, char *argv[]){
	int nproc;
	/* Two file descriptors for communication with the shell */
	static int request_fd, return_fd;
	int i;
	pid_t p;

	q = init_queue();
	/* Create the shell. */
	sched_create_shell(SHELL_EXECUTABLE_NAME, &request_fd, &return_fd);
	/* add the shell to the scheduler's tasks */
	/*
	 * For each of argv[1] to argv[argc - 1],
	 * create a new child process, add it to the process list.
	 */
	
	nproc = argc-1; /* number of proccesses goes here */

	if (nproc == 0) {
		fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
		exit(1);
	}
	
	struct process * proc;
	for (i = 0; i<nproc;i++) {
		p = fork();
		if (p < 0) {
			perror("fork");
			exit(1);
		}

		if (p == 0) {
			char *newargv[] = { argv[i+1], NULL, NULL, NULL };
        		char *newenviron[] = { NULL };
			raise(SIGSTOP);
			execve(argv[i+1],newargv,newenviron);
			exit(1);
		}
		proc = malloc(sizeof(struct process));
		proc->pid = p;
		id++;
		proc->myid = id;
		proc->name = argv[i+1];
		enqueue(proc,q);
		
	}
	
	/* Wait for all children to raise SIGSTOP before exec()ing. */
	wait_for_ready_children(nproc);

	/* Install SIGALRM and SIGCHLD handlers. */
	install_signal_handlers();

    	/* Arrange for an alarm after 1 sec */
    	if (alarm(SCHED_TQ_SEC) < 0) {
            perror("alarm");
            exit(1);
    	}	

	proc = get_top(q);
	kill(proc->pid,SIGCONT);

	shell_request_loop(request_fd, return_fd);

	/* Now that the shell is gone, just loop forever
	 * until we exit from inside a signal handler.
	 */
	while (pause())
		printf("to be continued\n");;
	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}
