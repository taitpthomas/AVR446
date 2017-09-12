/*
 * POSIX Real Time Example
 * using a single pthread as RT thread
 */

#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <sys/io.h>
#include <signal.h>
#include <stdbool.h>
#include "global.h"
#include "speed_cntr.h"

// Global status flags
struct GLOBAL_FLAGS status = {FALSE, FALSE, 0};

// Parallel Port
#define BASE 0x378
#define OUTB(a)	do { outb((a), BASE); } while (0)

int running = 1;

void signalHandler(int sig)
{
	running = 0;
}


struct period_info {
        struct timespec next_period;
        long period_ns;
};
 
static void inc_period(struct period_info *pinfo) 
{
        pinfo->next_period.tv_nsec += pinfo->period_ns;
 
        while (pinfo->next_period.tv_nsec >= 1000000000) {
                /* timespec nsec overflow */
                pinfo->next_period.tv_sec++;
                pinfo->next_period.tv_nsec -= 1000000000;
        }
}
 
static void periodic_task_init(struct period_info *pinfo)
{
        /* for simplicity, hardcoding a 1ms period */
        pinfo->period_ns = 1000000;
 
        clock_gettime(CLOCK_MONOTONIC, &(pinfo->next_period));
}
 
static void wait_rest_of_period(struct period_info *pinfo)
{
        inc_period(pinfo);
 
        /* for simplicity, ignoring possibilities of signal wakes */
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &pinfo->next_period, NULL);
}

void *simple_cyclic_task(void *data)
{
        struct period_info pinfo;
	int count = 0;
	int rc;
	int step_width = 0;
	
	printf("%s started\n", __FUNCTION__);	 
        periodic_task_init(&pinfo);
        while (running){
		count++;
		/* reset step clock pin to zero */
		if (step_width > 0){
			step_width--;
			if (step_width == 0){
				OUTB(0);
			}
		}
		/* timer/counter compare output */
                if (count > 1000){
			/* do realtime task */
			rc = speed_cntr_TIMER1_COMPA_interrupt();
			switch(rc){
				case NOACTION:
					break;
				case CW:
				case CCW:
					OUTB(0xff);
					step_width = 100;
					break;
			}
			count = 0;
		}
                wait_rest_of_period(&pinfo);
        }
 
        return NULL;
}
 
int main(int argc, char* argv[])
{
        struct sched_param param;
        pthread_attr_t attr;
        pthread_t thread;
        int ret;
	
	/* initialize parallel port */
	printf("Parallel Port Interface (Base: 0x%x)\n", BASE);
	
	// Set permission bits of 4 ports starting from BASE
	if (ioperm(BASE, 4, 1) != 0){
		printf("ERROR: Could not set permissions on ports");
		return 0;
	}

	/* ctrl-c handler */
	signal(SIGINT, signalHandler);

        /* Lock memory */
        if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
                printf("mlockall failed: %m\n");
		// Clear permission bits of 4 ports starting from BASE
		ioperm(BASE, 4, 0);
                exit(-2);
        }

        /* Initialize pthread attributes (default values) */
        ret = pthread_attr_init(&attr);
        if (ret) {
                printf("init pthread attributes failed\n");
                goto out;
        }
 
        /* Set a specific stack size  */
        ret = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
        if (ret) {
        	printf("pthread setstacksize failed\n");
		goto out;
        }
 
        /* Set scheduler policy and priority of pthread */
        ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        if (ret) {
                printf("pthread setschedpolicy failed\n");
                goto out;
        }
        param.sched_priority = 80;
        ret = pthread_attr_setschedparam(&attr, &param);
        if (ret) {
                printf("pthread setschedparam failed\n");
                goto out;
        }
        /* Use scheduling parameters of attr */
        ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        if (ret) {
                printf("pthread setinheritsched failed\n");
                goto out;
        }
 
        /* Create a pthread with specified attributes */
        ret = pthread_create(&thread, &attr, simple_cyclic_task, NULL);
        if (ret) {
                printf("create pthread failed\n");
                goto out;
        }
 
        /* Join the thread and wait until it is done */
        ret = pthread_join(thread, NULL);
        if (ret)
                printf("join pthread failed: %m\n");
 
out:
	// Clear permission bits of 4 ports starting from BASE
	ioperm(BASE, 4, 0);
        return ret;
}


