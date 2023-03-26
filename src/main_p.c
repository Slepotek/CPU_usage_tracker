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
#include "../lib/printer/printer.h"

sem_t empty, full, empty_r, full_r;

pthread_mutex_t conch, conch_r;

pthread_t reader, analyzer, printer; //thread declaration

an_args *pass;

volatile int process_end;

/// @brief Reader procedure - starts reading of the /proc/stat file, returns results to stats buffer and oversess access to stats buffer
/// @param buff buffer with stats structure
void reader_procedure(ring_buffer *buff)
{
    size_t proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN); //number of phisycial processors 
    //the clock is used to match exactly 1 second window of mesurement
    clock_t start, end; //clock variables
    double cpu_time_used; //time passed in the procedure 
    double one_sec = WINDOW_TIME; //window time in secs
    struct stats_cpu *temp_d = (struct stats_cpu*)malloc(sizeof(struct stats_cpu) * proc_num); //initialize the temp variable for data 
    //memset(temp_d, 0, sizeof(struct stats_cpu) * proc_num);
    while(1)
    {
        //simple thread break (cleanup is at the end of each procedure)
        if(process_end == 1)
        {
            break;
        }
        start = clock(); //start the clock count

        read_stats(temp_d); //trigger /proc/stat file reading (no access controll - used only by one thread)

        sem_wait(&empty);//set semaphore (decrement number of empty slots in buffer)
        pthread_mutex_lock(&conch); //set mutex 
        ring_buffer_push(buff, temp_d);//put data into buffer
        pthread_mutex_unlock(&conch);//unlock mutex
        sem_post(&full);//set semaphore (increment the number of full bufer slots)

        end = clock();//stop the clock
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; //compute the overal time that reading file and pushing to buffer took
        if(cpu_time_used <= one_sec)
        {
            usleep((__useconds_t)((one_sec-cpu_time_used)*100000)); //sleep for the period left to 1 sec if ever less than 1 sec
        }
    }
    free(temp_d);//free alocated memory if the thread stops
    printf("Reader is dropping execution\n");
}


/// @brief Analyzer procedure - starts stats computations and oversses the buffer's access
/// @param args structure with pointers to stats buffer and results buffer
void analyze_stats (an_args *args)
{   
    size_t proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN); //number of phisycial processors 

    size_t var_size = (size_t)(sizeof(struct stats_cpu) * (proc_num)); //size of the aray of stats_cpu structures
    u_int *results = (u_int *)malloc(sizeof(u_int) * proc_num); //allocate results array
    struct stats_cpu *prev = (struct stats_cpu *)malloc(var_size); //allocate previous stats array
    struct stats_cpu *curr = (struct stats_cpu *)malloc(var_size); //allocate current stats array
    initialize_stats_var(prev, 0);//initialize first array to 0's
    initialize_stats_var(curr, 1);//second is initialized to 1 to avoid undefined behaviour
    clock_t start, end;
    double cpu_time_used; //time passed in the procedure 
    double one_sec = WINDOW_TIME; //window time in secs
    while (1)
    {
        //simple thread break (cleanup is at the end of each procedure)
        if(process_end == 1)
        {
            break;
        }
        start = clock();
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

        end = clock();//stop the clock
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; //compute the overal time that reading file and pushing to buffer took
        if(cpu_time_used <= one_sec)
        {
            usleep((__useconds_t)((one_sec-cpu_time_used)*100000)); //sleep for the period left to 1 sec
        }
    }
    //free allocated memory 
    free(results);
    free(prev);
    free(curr);
    printf("Analyzer is finishing its execution \n");
}

/// @brief Printer procedure - prints the values of percentage for each CPU to the console
/// @param buff u_int array type that stores percentage values
void print_stats(ring_buffer *buff)
{
    size_t proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN); //number of phisycial processors 
    clock_t start, end;
    double cpu_time_used; //time passed in the procedure 
    double one_sec = WINDOW_TIME; //window time in secs
    u_int *toPrint = (u_int *)malloc(sizeof(u_int) * proc_num); //allocate results array

    printf("CPUsageTracker - mesures CPU usage over time\n");
    printf("Mesurement of stats is approximately in 1 sec intervals\n");
    printf("============================================================\n");
    printf("=                                                           \n");
    while(1)
    {
        //simple thread break (cleanup is at the end of each procedure)
        if(process_end == 1)
        {
            break;
        }
        start = clock();
        //read from results buffer
        sem_wait(&full_r);
        pthread_mutex_lock(&conch_r);
        ring_buffer_pop(buff, toPrint);
        pthread_mutex_unlock(&conch_r);
        sem_post(&empty_r);

        if(*toPrint > 1)//print the results
        {
            print_the_results(toPrint);
        }

        end = clock();//stop the clock
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; //compute the overal time that reading file and pushing to buffer took
        if(cpu_time_used <= one_sec)
        {
            usleep((__useconds_t)((one_sec-cpu_time_used)*100000)); //sleep for the period left to 1 sec
        }
    }
    free(toPrint);
}

/// @brief Initializes stats_cpu structure to given "val" value
/// @param var pointer to stats_cpu structure
/// @param val value to witch initialize structure fields
void initialize_stats_var (struct stats_cpu *var, u_ll val)
{

    size_t proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN); //number of phisycial processors 

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
