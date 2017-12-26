/*
 * client_server.c
 *
 *  Created on: Oct 20, 2016
 *      Author: Nathan J Thomas
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#include "client_server.h"

const char option1[] = "--servers";
const char option2[] = "--clients";
const char option3[] = "--mu";
const char option4[] = "--lambda";
double MU = 0.01;
double LAMBDA = 0.005;
int NUM_SERVER = 2;
int NUM_CLIENT = 2;
int INACTIVE = 0;
int READY = 0;
int QUEUE = 0;
int FINISHED = 1;
double AWT = 0;		/* Average Wait Time */
double ATA = 0;		/* Average Turnaround Time */
double AXT = 0;		/* Average Execution Time */
double AIA = 0;		/* Average Interarrival Time */
double AQL = 0;		/* Average Queue Length */
double AAT = 0; 	/* Average Arrival Time */
int JOBS_COMPLETED = 0;
int TOTAL_JOBS = 0;
int TICKS_SO_FAR = 0;
int *arrivaltimes, *completiontimes;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t clock_cond = PTHREAD_COND_INITIALIZER;

/************************************************************************/
/**********                        client                      **********/
/************************************************************************/

void *client(void* arg){
	double random;
	struct timeval ti;

	while(FINISHED){
		if(pthread_mutex_lock(&count_mutex) != 0){
			fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
			exit(1);
		}
		READY++;
		if(pthread_cond_wait(&cond, &count_mutex) != 0){
			fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
			exit(1);
		}
		gettimeofday(&ti, NULL);	
		srand(ti.tv_usec);
		random = ((double)rand()) / ((double)RAND_MAX);
		if (random < LAMBDA){
			QUEUE++;
			arrivaltimes[TOTAL_JOBS] = TICKS_SO_FAR;
			TOTAL_JOBS++;
		}
		INACTIVE++;
		if (INACTIVE >= NUM_SERVER+ NUM_CLIENT){
			if(pthread_cond_signal(&clock_cond) != 0){
				fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
				exit(1);
			}
		}
		if(pthread_mutex_unlock(&count_mutex) != 0){
			fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
			exit(1);
		}
	}
	pthread_exit(0);
}

/************************************************************************/
/**********                        server                      **********/
/************************************************************************/

void *server(void* arg){
	int status, burst;
	double random;
	struct timeval ti;

	status = FREE;
	while(FINISHED){
		if(pthread_mutex_lock(&count_mutex) != 0){
			fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
			exit(1);
		}
		READY++;
		if(pthread_cond_wait(&cond, &count_mutex) != 0){
			fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
			exit(1);
		}
		if (status == FREE){
			if (QUEUE > 0){
				QUEUE--;
				burst = 0;
				status = BUSY;
			}					
		} else {
			burst++;
			gettimeofday(&ti, NULL);	
			srand(ti.tv_usec);
			random = ((double)rand()) / ((double)RAND_MAX);
			if (random <= MU){
				status = FREE;
				completiontimes[JOBS_COMPLETED] = TICKS_SO_FAR;
				JOBS_COMPLETED++;
				AXT += burst;
			}
		}
		INACTIVE++;
		if (INACTIVE >= NUM_SERVER+ NUM_CLIENT){
			if(pthread_cond_signal(&clock_cond) != 0){
				fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
				exit(1);
			}
		}
		if(pthread_mutex_unlock(&count_mutex) != 0){
			fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
			exit(1);
		}
	}
	pthread_exit(0);
}

/************************************************************************/
/**********                        clock1                      **********/
/************************************************************************/

void *clock1(void* arg){
	for (; TICKS_SO_FAR < TICK; TICKS_SO_FAR++){
		while(READY < NUM_SERVER+NUM_CLIENT)
			; /* do nothing */
		if(pthread_mutex_lock(&count_mutex) != 0){
			fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
			exit(1);
		}
		READY = 0;
		AQL += QUEUE;
		if (pthread_cond_broadcast(&cond) != 0){
			fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
			exit(1);
		}
		if (pthread_cond_wait(&clock_cond, &count_mutex) != 0){
			fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
			exit(1);
		}
		INACTIVE = 0;
		if(pthread_mutex_unlock(&count_mutex) != 0){
			fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
			exit(1);
		}
	}
	FINISHED = 0;
	if (pthread_cond_broadcast(&cond) != 0){
		fprintf(stderr, "%main: pthread_join failed: %s\n", strerror(errno));
		exit(1);
	}
	pthread_exit(0);
}

/************************************************************************/
/**********                        main                        **********/
/************************************************************************/

int main(int argc, char *argv[]){
	int i;
	for (i = 1; i < argc; i+= 2){
		if (strcmp(argv[i], option1) == 0){
			NUM_SERVER = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], option2) == 0){
			NUM_CLIENT = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], option3) == 0){
			MU = strtod(argv[i + 1], NULL);
		} else if (strcmp(argv[i], option4) == 0){
			LAMBDA = strtod(argv[i + 1], NULL);
		} else {
			fprintf(stderr,"%s is not an option\n", argv[i]);
			i = argc;
		}
	}

	pthread_t *server_tid, *client_tid, clock_tid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);

	server_tid = malloc(NUM_SERVER * sizeof(pthread_t));
	client_tid = malloc(NUM_CLIENT * sizeof(pthread_t));
	arrivaltimes = calloc(TICK*NUM_CLIENT, sizeof(int));
	completiontimes = calloc(TICK*NUM_CLIENT, sizeof(int));

	if (pthread_create(&clock_tid, &attr, clock1, NULL) != 0){
		fprintf(stderr, "%s: pthread_create failed: %s\n", argv[0], strerror(errno));
		exit(1);
	}
	int h, j, k, l;
	for (l = 0; l < NUM_SERVER; l++){
		if (pthread_create(&server_tid[l], &attr, server, NULL) != 0){
			fprintf(stderr, "%s: pthread_create failed: %s\n", argv[0], strerror(errno));
			exit(1);
		}
	}
	for(h = 0; h < NUM_CLIENT; h++){
		if (pthread_create(&client_tid[h], &attr, client, NULL) != 0){
			fprintf(stderr, "%s: pthread_create failed: %s\n", argv[0], strerror(errno));
			exit(1);
		}
	}
	for (j = 0; j < NUM_SERVER; j++){
		if (pthread_join(server_tid[j], NULL) != 0){
			fprintf(stderr, "%s: pthread_join failed: %s\n", argv[0], strerror(errno));
			exit(1);
		}
	}
	for (k = 0; k < NUM_CLIENT;k++){
		if (pthread_join(client_tid[k], NULL) != 0){
			fprintf(stderr, "%s: pthread_join failed: %s\n", argv[0], strerror(errno));
			exit(1);
		}
	}
	if (pthread_join(clock_tid, NULL) != 0){
		fprintf(stderr, "%s: pthread_join failed: %s\n", argv[0], strerror(errno));
		exit(1);
	}
	int g;
	for (g = 0; g < JOBS_COMPLETED; g++){
		ATA += (completiontimes[g] - arrivaltimes[g]);
	}
	ATA = ATA / JOBS_COMPLETED;
	AXT = (AXT / JOBS_COMPLETED);
	AAT = ((double)TOTAL_JOBS / (double)TICK);
	AIA = 1 / AAT;
	AWT = ATA - AXT;
	printf("Jobs created: %d\n", TOTAL_JOBS);
	printf("Jobs completed: %d\n", JOBS_COMPLETED);
	printf("Average waiting time (AWT): %f cycles\n", AWT);
	printf("Average execution time (AXT): %f cycles\n", AXT);
	printf("Average turnaround time (ATA): %f cycles\n", ATA); // Average completion time - Arrival rate
	printf("Average interarrivaltime (AIA): %f cycles\n", AIA);
	printf("Average queue length (AQL): %d\n", (int)(AQL / TICK));
	return 0;
}


