/*
 * mandel.c
 *
 * A program to draw the Mandelbrot Set on a 256-color xterm.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#define perror_pthread(ret,msg) \
        do ( errno = ret; perror(msg) ; ) while (0)

/***************************
 * Compile-time parameters *
 ***************************/

/*
 * Output at the terminal is is x_chars wide by y_chars long
*/
int y_chars = 50;
int x_chars = 90;

/*
 * The part of the complex plane to be drawn:
 * upper left corner is (xmin, ymax), lower right corner is (xmax, ymin)
*/
double xmin = -1.8, xmax = 1.0;
double ymin = -1.0, ymax = 1.0;
	
/*
 * Every character in the final output is
 * xstep x ystep units wide on the complex plane.
 */
double xstep;
double ystep;


//semaphores array
sem_t *sem_arr;

// The number of threads I give
int NTHREADS;

struct *counter {
    	int i;
}

/*
 * This function computes a line of output
 * as an array of x_char color values.
 */
void compute_mandel_line(int line, int color_val[])
{
	/*
	 * x and y traverse the complex plane.
	 */
	double x, y;

	int n;
	int val;

	/* Find out the y value corresponding to this line */
	y = ymax - ystep * line;

	/* and iterate for all points on this line */
	for (x = xmin, n = 0; n < x_chars; x+= xstep, n++) {

		/* Compute the point's color value */
		val = mandel_iterations_at_point(x, y, MANDEL_MAX_ITERATION);
		if (val > 255)
			val = 255;

		/* And store it in the color_val[] array */
		val = xterm_color(val);
		color_val[n] = val;
	}
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int color_val[])
{
	int i;
	
	char point ='@';
	char newline='\n';

	for (i = 0; i < x_chars; i++) {
		/* Set the current color, then output the point */
		set_xterm_color(fd, color_val[i]);
		if (write(fd, &point, 1) != 1) {
			perror("compute_and_output_mandel_line: write point");
			exit(1);
		}
	}

	/* Now that the line is done, output a newline character */
	if (write(fd, &newline, 1) != 1) {
		perror("compute_and_output_mandel_line: write newline");
		exit(1);
	}
}

void *compute_and_output_mandel_line(struct counter arg)
{
	/*
	 * A temporary array, used to hold color values for the line being drawn
	 */
	int color_val[x_chars], fd = 1, line;


	for(line = arg->i; line <= y_chars; line += NTHREADS){
                compute_mandel_line(line, color_val);
                sem_wait(&sem_arr[arg->i]);
                output_mandel_line(fd, color_val);

                if(arg->i == NTHREADS-1){
                        sem_post(&sem_arr[0]);
                }
                else {
                        sem_post(&sem_arr[(arg->i)+1]);
                }
        }
}

int main(int argc, char *argv[])
{
	int line;

	xstep = (xmax - xmin) / x_chars;
	ystep = (ymax - ymin) / y_chars;

	if(argc!=2){
                printf("Usage: %s Number of threads\n ", argv[0]);
                exit(1);
    }

    NTHREADS = atoi(argv[1]);
    int ret;
    pthread_t *t;
    struct counter count;
    int i;

    sem_arr = malloc(NTHREADS*sizeof(sem_t)); //array of semaphores as many as the threads
        //thread_id = malloc(thread_num*sizeof(sem_t));
    t = malloc(NTHREADS*sizeof(pthread_t));

	/*
	 * draw the Mandelbrot Set, one line at a time.
	 * Output is sent to file descriptor '1', i.e., standard output.
	 */
	for (i = 0; i < NTHREADS; i++) {
		sem_init(&sem_arr[count->i], 0 , 0);
        ret = pthread_create(&t[count->i], NULL, compute_and_output_mandel_line, *i);
        if(ret){
                perror("pthread_create");
                exit(1);
        }
	}

	 /* Unlock the first semaphore */
    sem_post(&sem_arr[0]);
	for (i = 0; i < NTHREADS; i++){
		ret = pthread_join(t[count->i], NULL);
                if(ret){
                        perror("pthread_join");
                        exit(1);
                }
                sem_destroy(&sem_arr[i]);
        }
        free (sem_arr);
        free(t);
        reset_xterm_color(1);
	return 0;
}
