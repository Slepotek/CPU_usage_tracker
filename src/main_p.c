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

static ring_buffer* stat_buffer; //buffer for stats
static ring_buffer* res_buffer; //buffer for results

static sem_t empty, full, empty_r, full_r;

static pthread_mutex_t conch, conch_r;

pthread_t reader, analyzer, printer, logger, watchdog, inputer; //thread declaration

static an_args *pass;

static time_t reader_last_activity, printer_last_activity, analyzer_last_activity, logger_last_activity;

static volatile sig_atomic_t t = 1;
static volatile sig_atomic_t w = 1;

static struct sigaction action;//decalre sigaction structure (for SIGTERM)

void terminate (int sigint) //leaved to expand functionality of this app
{
    t = 0;
    w = 0;
}

void destroy_leftovers(void)
{
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
}

void initialize_program_variables(void)
{
    printf("Enetring initialization process\n");

    memset(&action, 0, sizeof(struct sigaction));//clear the sigaction structure memory
    action.sa_handler = terminate; //assign a function to this action handler
    sigaction(SIGTERM, &action, NULL);//assign a signal value to the action structure
    
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
    
    //stats_cpu buffer sem and mutex
    sem_init(&empty, 0, BUFFER_SIZE - 1); //initialize the semaphore variable (empty slots in buffer)
    sem_init(&full, 0, 0); //initialize the semaphore variable (full slots in buffer)
    pthread_mutex_init(&conch, NULL); //initialize the mutex

    //results buffer sem and mutex
    sem_init(&empty_r, 0, BUFFER_SIZE - 1); //initialize the semaphore variable (empty slots in buffer)
    sem_init(&full_r, 0, 0); //initialize the semaphore variable (full slots in buffer)
    pthread_mutex_init(&conch_r, NULL); //initialize the mutex
}

/// @brief Reader procedure - starts reading of the /proc/stat file, returns results to stats buffer and oversess access to stats buffer
/// @param buff buffer with stats structure
void reader_procedure(ring_buffer *buff)
{   
    //thread timeout initialization
    struct timespec r_timeout;

    size_t proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN); //number of phisycial processors 
    //the clock is used to match exactly 1 second window of mesurement
    clock_t start, end; //clock variables
    double cpu_time_used; //time passed in the procedure 
    double one_sec = WINDOW_TIME; //window time in secs
    struct stats_cpu *temp_d = (struct stats_cpu*)malloc(sizeof(struct stats_cpu) * proc_num); //initialize the temp variable for data 
    //memset(temp_d, 0, sizeof(struct stats_cpu) * proc_num);
    while(t)
    {
        reader_last_activity = time(NULL);
        clock_gettime(CLOCK_REALTIME, &r_timeout);
        r_timeout.tv_sec += THREAD_TIMEOUT;
        //simple thread break (cleanup is at the end of each procedure)
        start = clock(); //start the clock count

        read_stats(temp_d); //trigger /proc/stat file reading (no access controll - used only by one thread)

        sem_wait(&empty);//set semaphore (decrement number of empty slots in buffer)
        if(pthread_mutex_timedlock(&conch, &r_timeout) == 0) //set mutex 
        {
            ring_buffer_push(buff, temp_d);//put data into buffer
            pthread_mutex_unlock(&conch);//unlock mutex
        }
        else
        {
            pthread_mutex_unlock(&conch);
            //TODO:Log message that thread timed out while trying to acquire mutex
        }
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
    //thread timeout init
    struct timespec a_timeout; //special structure for timeout

    //helper variables init
    size_t proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN); //number of phisycial processors 
    size_t var_size = (size_t)(sizeof(struct stats_cpu) * (proc_num)); //size of the aray of stats_cpu structures
    u_int *results = (u_int *)malloc(sizeof(u_int) * proc_num); //allocate results array
    struct stats_cpu *prev = (struct stats_cpu *)malloc(var_size); //allocate previous stats array
    struct stats_cpu *curr = (struct stats_cpu *)malloc(var_size); //allocate current stats array
    initialize_stats_var(prev, 0);//initialize first array to 0's
    initialize_stats_var(curr, 1);//second is initialized to 1 to avoid undefined behaviour
    clock_t start, end; //clock variables
    double cpu_time_used; //time passed in the procedure 
    double one_sec = WINDOW_TIME; //window time in secs

    //main analyzer thread loop
    while (t)
    {
        analyzer_last_activity = time(NULL);
        clock_gettime(CLOCK_REALTIME, &a_timeout); //acquire current time
        a_timeout.tv_sec += THREAD_TIMEOUT; //add thread timeout time to current time
        start = clock();
        //stats buffer operation
        sem_wait(&full);//set semaphore (decrement number of full slots in stats_buffer)
        if(pthread_mutex_timedlock(&conch, &a_timeout) == 0)
        {
            ring_buffer_pop(args->stat_buff, curr);//get the data from the stats buffer
            pthread_mutex_unlock(&conch);
        }
        else
        {
            pthread_mutex_unlock(&conch);
            //TODO:Log message that thread timed out while trying to acquire mutex
        }
        sem_post(&empty);

        //computations
        analyzer_compute(prev, curr, results);//analyze stats

        //results buffer operation
        sem_wait(&empty_r);
        if(pthread_mutex_timedlock(&conch_r, &a_timeout) == 0)
        {
            ring_buffer_push(args->res_buff, results);//put data into results buffer
            pthread_mutex_unlock(&conch_r);
        }
        else
        {
            pthread_mutex_unlock(&conch_r);
            //TODO:Log message that thread timed out while trying to acquire mutex
        }
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
    struct timespec p_timeout;
    size_t proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN); //number of phisycial processors 
    clock_t start, end;
    double cpu_time_used; //time passed in the procedure 
    double one_sec = WINDOW_TIME; //window time in secs
    u_int *toPrint = (u_int *)malloc(sizeof(u_int) * proc_num); //allocate results array
    printf("CPUsageTracker - mesures CPU usage over time\n");
    printf("Mesurement of stats is approximately in 1 sec intervals\n");
    printf("============================================================\n");
    printf("=                                                           \n");
    while(t)
    {
        printer_last_activity = time(NULL);
        clock_gettime(CLOCK_REALTIME, &p_timeout);
        p_timeout.tv_sec += THREAD_TIMEOUT;
        start = clock();
        //read from results buffer
        sem_wait(&full_r);
        if(pthread_mutex_timedlock(&conch_r, &p_timeout) == 0)
        {
            ring_buffer_pop(buff, toPrint);
            pthread_mutex_unlock(&conch_r);
        }
        else
        {
            pthread_mutex_unlock(&conch_r);
            //TODO: Log message that thread timed out while trying to acquire mutex
        }
        sem_post(&empty_r);

        if(*toPrint > 1)//print the results (left from debugging but after second thought could be useful)
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

void watchdog_watch(void)
{
    while(w)
    {
        time_t timestamp = time(NULL);
        if(timestamp - reader_last_activity > TIMEOUT )
        {
            //this will gently close the application
            terminate(1);
        }
        if(timestamp - analyzer_last_activity > TIMEOUT )
        {
            //this will gently close the application
            terminate(1);
        }
        if(timestamp - printer_last_activity > TIMEOUT )
        {
            //this will gently close the application
            terminate(1);
        }
    }
    pthread_join(reader, NULL);
    pthread_join(analyzer, NULL);
    pthread_join(printer, NULL);
    pthread_cancel(inputer);
    printf("Application closed due to thread time out.\n");
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

void inputer_check(void)
{
    getchar();
    terminate(2);
}

