//This app only counts physical processors, if on VM then only VM processors
//It is not compatible with processor hot plugin, althought not likely the possibility always exists. 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include "main_p.h"
#include "main.h"
#include "structures.h"
#include "../lib/reader/reader.h"
#include "../lib/buffer/buffer.h"
#include "../lib/analyzer/analyzer.h"

static ring_buffer* stat_buffer; //buffer for stats
static ring_buffer* res_buffer; //buffer for results

extern sem_t empty, full, empty_r, full_r;

extern pthread_mutex_t conch, conch_r;

extern pthread_t reader, analyzer, printer; //thread declaration

extern an_args *pass;
//extern volatile int process_end;
extern volatile sig_atomic_t t;

void terminate (int val) //leaved to expand functionality of this app
{
    t = 0;
}

int main(void)
{
    //process_end = 0;
    printf("Enetring initialization process\n");

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = terminate;
    sigaction(SIGTERM, &action, NULL);
    
    //buffers initialization
    printf("Allocating the buffer\n");

    stat_buffer = (ring_buffer*)malloc(sizeof(ring_buffer));//memmory allocation
    if(stat_buffer == NULL)//check if the memmory was alocated
    {
        perror("NULL exception in malloc\n");
    }
    stats_ring_buffer_init(stat_buffer); //initialize stats ring buffer

    res_buffer = (ring_buffer*)malloc(sizeof(ring_buffer));//memmory allocation
    if(res_buffer == NULL)//check if the memory was alocated
    {
        perror("NULL exception in malloc\n");
    }
    res_ring_buffer_init(res_buffer);//initialize results ring buffer

    pass = (an_args*)malloc(sizeof(an_args));//allocate memory for args structure (arguments for analyze_stats procedure)
    if(pass == NULL)//check if the memory was alocated
    {
        perror("NULL exception in malloc\n");
    }
    pass->res_buff = res_buffer;//assign res_buffer pointer to args
    pass->stat_buff = stat_buffer;//assign stat_buffer pointer to args
    //buffers initialization end
    
    sem_init(&empty, 0, BUFFER_SIZE - 1); //initialize the semaphore variable (empty slots in buffer)
    sem_init(&full, 0, 0); //initialize the semaphore variable (full slots in buffer)
    pthread_mutex_init(&conch, NULL); //initialize the mutex

    sem_init(&empty_r, 0, BUFFER_SIZE - 1); //initialize the semaphore variable (empty slots in buffer)
    sem_init(&full_r, 0, 0); //initialize the semaphore variable (full slots in buffer)
    pthread_mutex_init(&conch_r, NULL); //initialize the mutex

    pthread_create(&reader, NULL, read_proc , NULL); //run the reader thread (the warning about bad conversion type can be overcomed by creating explicit function calling the base function - redundant code)
    pthread_create(&analyzer, NULL, analyze_proc, NULL);//run analyzer thread
    pthread_create(&printer, NULL, print_proc, NULL);//run print thread

    getchar();
    terminate(1);

    //wait for the thread to finish
    pthread_join(reader, NULL);
    pthread_join(analyzer, NULL);
    pthread_join(printer, NULL);
    
    printf("Gentle closing finished \n");

    //destroy semaphores and mutex
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&conch);

    sem_destroy(&empty_r);
    sem_destroy(&full_r);
    pthread_mutex_destroy(&conch_r);

    //free alocated memory
    ring_buffer_free(stat_buffer);
    ring_buffer_free(res_buffer);
    free(pass);

    printf("Resources freed\n");
    printf("Closing...\n");
    return 0;
}

void* read_proc (void *s)
{
    reader_procedure(stat_buffer);
    return s;
}

void* analyze_proc (void *s)
{
    analyze_stats(pass);
    return s;
}

void* print_proc (void *s)
{
    print_stats(res_buffer);
    return s;
}
