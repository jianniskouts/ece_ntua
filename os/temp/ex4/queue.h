#ifndef QUEUE_H_
#define QUEUE_H_

//typedef struct queue queue;

struct process {
	pid_t pid;
	int myid;
	char * name;
	int prio;
};

typedef struct node {
	struct process * p;
	struct node * pre;
} node;

typedef struct queue {
	struct node * head;
	struct node * tail;
	int size;
} queue;

queue * init_queue() ;

struct process * dequeue(queue * q);

struct process * get_top(queue *q);

void enqueue(struct process * p,queue * q);

char * name_by_pid(pid_t p,queue * q);

#endif /* REQUEST_H_ */

