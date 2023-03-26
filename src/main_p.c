#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include "main_p.h"
#include "../lib/reader/reader.h"
#include "../lib/buffer/buffer.h"
#include "../lib/analyzer/analyzer.h"


/// @brief Reader procedure - starts reading of the /proc/stat file, returns results to stats buffer and oversess access to stats buffer
/// @param buff buffer with stats structure
/// @return Returns stats structure to stats buffer
void *reader_procedure(ring_buffer *buff)
{
    //the clock is used to match exactly 1 second window of mesurement
    clock_t start, end; //clock variables
    double cpu_time_used; //time passed in the procedure 
    double one_sec = 1; //one second variable
    struct stats_cpu *temp_d = malloc(sizeof(struct stats_cpu) * (proc_num)); //initialize the temp variable for data 
    while(1)
    {
        start = clock(); //start the clock count

        read_stats(temp_d); //trigger /proc/stat file reading (no access controll - used only by one thread)

        sem_wait(&empty);//set semaphore (decrement number of empty slots in buffer)
        pthread_mutex_lock(&conch); //set mutex 
        ring_buffer_push(buff, temp_d);//put data into buffer
        pthread_mutex_unlock(&conch);//unlock mutex
        sem_post(&full);//set semaphore (increment the number of full bufer slots)

        memset(temp_d, 0, sizeof(struct stats_cpu) * (proc_num)); //zeroize temp data struct

        end = clock();//stop the clock
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; //compute the overal time that reading file and pushing to buffer took
        if(cpu_time_used <= one_sec)
        {
            usleep((__useconds_t)((one_sec-cpu_time_used)*1000)); //sleep for the period left to 1 sec if ever less than 1 sec
        }
    }
    free(temp_d);//free alocated memory if the thread stops
    printf("Reader is dropping execution\n");
}

/// @brief Analyzer procedure - starts stats computations and oversses the buffer's access
/// @param args structure with pointers to stats buffer and results buffer
/// @return Returns percentage value of computation results to results buffer
void *analyze_stats (an_args *args)
{   
    size_t var_size = (size_t)(sizeof(struct stats_cpu) * (proc_num)); //size of the aray of stats_cpu structures
    u_int *results = (u_int *)malloc(sizeof(u_int) * proc_num); //allocate results array
    struct stats_cpu *prev = (struct stats_cpu *)malloc(var_size); //allocate previous stats array
    struct stats_cpu *curr = (struct stats_cpu *)malloc(var_size); //allocate current stats array
    initialize_stats_var(prev, 0);//initialize first array to 0's
    initialize_stats_var(curr, 1);//second is initialized to 1 to avoid undefined behaviour
    while (1)
    {

        //stats buffer operation
        sem_wait(&full);//set semaphore (decrement number of full slots in stats_buffer)
        pthread_mutex_lock(&conch);
        ring_buffer_pop(args->stat_buff, curr);//get the data from the stats buffer
        pthread_mutex_unlock(&conch);
        sem_post(&empty);

        //computations
        analyzer_compute(prev, curr, results);//analyze stats

        //results buffer operation
        sem_wait(&empty_r);
        pthread_mutex_lock(&conch_r);
        ring_buffer_push(args->res_buff, results);//put data into results buffer
        pthread_mutex_unlock(&conch_r);
        sem_post(&full_r);

        //variables switching
        memcpy(prev, curr, var_size);
    }
    //free allocated memory 
    free(results);
    free(prev);
    free(curr);
    printf("Printer is finishin its execution \n");
}

/// @brief Initializes stats_cpu structure to given "val" value
/// @param var pointer to stats_cpu structure
/// @param val value to witch initialize structure fields
void initialize_stats_var (struct stats_cpu *var, u_ll val)
{
    struct stats_cpu *idata; //interfacing pointer
    idata = var;
    for(int i = 1; i <= (int)proc_num; i++)
    {
        idata->cpu_user = val;
        idata->cpu_nice = val;
        idata->cpu_sys = val;
        idata->cpu_idle = val;
        idata->cpu_iowait = val;
        idata->cpu_hardirq = val;
        idata->cpu_softirq = val;
        idata->cpu_steal = val;
        idata->cpu_guest = val;
        idata->cpu_guest_nice = val;

        idata = var + i;
    }
}