#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include "main.h"
#include "../lib/reader/reader.h"
#include "../lib/buffer/buffer.h"


static sem_t empty, full;
static pthread_mutex_t conch;

static pthread_t reader, analyzer, printer; //thread declaration

int main(void)
{
    ring_buffer* buffer = malloc(sizeof(ring_buffer) + ((sizeof(struct stats_cpu) * (u_long)sysconf(_SC_NPROCESSORS_ONLN)) * BUFFER_SIZE)); //buffer variable
    ring_buffer_init(buffer); //initialize the ring buffer
 

    sem_init(&empty, 0, BUFFER_SIZE); //initialize the semaphore variable (empty slots in buffer)
    sem_init(&full, 0, 0); //initialize the semaphore variable (full slots in buffer)
    pthread_mutex_init(&conch, NULL); //initialize the mutex

    pthread_create(&reader, NULL, &reader_procedure, buffer); //run the reader thread (the warning about bad conversion type can be overcomed by creating explicit function calling the base function - redundant code)
    //pthread_create(&analyzer, NULL, analyze_stats, &)

    //wait for the thread to finish
    pthread_join(reader, NULL);
    
    //destroy semaphores and mutex
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&conch);
    }

static void *reader_procedure(ring_buffer *buff)
    {
    //the clock is used to match exactly 1 second window of mesurement
    clock_t start, end; //clock variables
    double cpu_time_used; //time passed in the procedure 
    double one_sec = 1; //one second variable
    struct stats_cpu *temp_d = malloc(sizeof(struct stats_cpu) * (u_long)(sysconf(_SC_NPROCESSORS_ONLN))); //initialize the temp variable for data 
    while(1)
    {
        start = clock(); //start the clock count
        read_stats(temp_d); //trigger /proc/stat file reading (no access controll - used only by one thread)
        sem_wait(&empty);//set semaphore (decrement number of empty slots in buffer)
        pthread_mutex_lock(&conch); //set mutex 
        ring_buffer_push(buff, temp_d);//put data into buffer
        pthread_mutex_unlock(&conch);//unlock mutex
        sem_post(&full);//set semaphore (increment the number of full bufer slots)
        memset(temp_d, 0, sizeof(struct stats_cpu) * (u_long)(sysconf(_SC_NPROCESSORS_ONLN)+1)); //zeroize temp data struct
        end = clock();//stop the clock
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; //compute the overal time that reading file and pushing to buffer took
        if(cpu_time_used <= one_sec)
        {
            usleep((__useconds_t)((one_sec-cpu_time_used)*1000)); //sleep for the period left to 1 sec if ever less than 1 sec
        }
    }
    }

