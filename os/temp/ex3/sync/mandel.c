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

/*Global Variables*/
int thread_num;
sem_t *sem_arr;

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

void *compute_and_output_mandel_line(void *arg)
{
        /*
         * A temporary array, used to hold color values for the line being drawn
         */
        int color_val[x_chars], line, fd = 1;
        volatile int *cur_thread = arg;
        for(line = *cur_thread; line<=y_chars; line+=thread_num){
                compute_mandel_line(line, color_val);
                sem_wait(&sem_arr[*cur_thread]);
                output_mandel_line(fd, color_val);

                if(*cur_thread == thread_num-1){
                        sem_post(&sem_arr[0]);
                }
                else {
                        sem_post(&sem_arr[(*cur_thread)+1]);
                }
        }

return NULL;
}

int main(int argc, char *argv[])
{
        int thread_counter, *thread_id, ret;
        pthread_t *t;

        xstep = (xmax - xmin) / x_chars;
        ystep = (ymax - ymin) / y_chars;

        /* checking if input is correct */
        if(argc!=2){
                printf("Usage: %s Number of threads\n ", argv[0]);
                exit(1);
        }

        thread_num = atoi(argv[1]);
        sem_arr = malloc(thread_num * sizeof(sem_t));
        thread_id = malloc(thread_num * sizeof(sem_t));
        t = malloc(thread_num * sizeof(pthread_t));
        /*                                                                                                                                                                          
         * draw the Mandelbrot Set, one line at a time.
         * Output is sent to file descriptor '1', i.e., standard output.
         */

        for (thread_counter = 0; thread_counter < thread_num; thread_counter++){
                thread_id[thread_counter] = thread_counter;
                sem_init(&sem_arr[thread_counter], 0 , 0);
                ret = pthread_create(&t[thread_counter], NULL, compute_and_output_mandel_line, &thread_id[thread_counter]);
                if(ret){
                        perror("pthread_create");
                        exit(1);
                }
        }
        /* Unlock the first semaphore */
        sem_post(&sem_arr[0]);
        for (thread_counter = 0; thread_counter < thread_num; thread_counter++){
                ret = pthread_join(t[thread_counter], NULL);
                if(ret){
                        perror("pthread_join");
                        exit(1);
                }
                sem_destroy(&sem_arr[thread_counter]);
        }
        free (sem_arr);
        free(t);
        reset_xterm_color(1);
        return 0;
}                                                                                                                                                
