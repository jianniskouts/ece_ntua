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

/*the process queue*/
queue * q;

struct process {

	pid_t pid;
	char * name;

};
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
	
	struct process *  p;
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




int main(int argc, char *argv[])
{
	int nproc;
	/*
	 * For each of argv[1] to argv[argc - 1],
	 * create a new child process, add it to the process list.
	 */
	int i;
	pid_t p;
	nproc = argc-1; /* number of proccesses goes here */
	if (nproc == 0) {
		fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
		exit(1);
	}
	q = init_queue();
	struct process * proc;
	for (i=0; i<nproc; i++) {
		p = fork();
		if (p < 0) {
			perror("fork");
			exit(1);
		}

		if (p == 0) {
			char *newargv[] = { argv[i], NULL, NULL, NULL };
        		char *newenviron[] = { NULL };
			raise(SIGSTOP);
			execve(argv[i+1],newargv,newenviron);
			exit(1);
		}
		proc = malloc(sizeof(struct process));
		proc->pid = p;
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


	/*start the process*/
	proc = get_top(q);
	kill(proc->pid,SIGCONT);


	/* loop forever  until we exit from inside a signal handler. */
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}