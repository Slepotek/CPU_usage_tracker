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
#include "../lib/logger/logger.h"

static ring_buffer* stat_buffer; //buffer for stats
static ring_buffer* res_buffer; //buffer for results

static sem_t empty, full, empty_r, full_r;

static pthread_mutex_t conch, conch_r;

pthread_t reader, analyzer, printer, logger, watchdog; //thread declaration

static an_args *pass;

static time_t reader_last_activity, printer_last_activity, analyzer_last_activity;
extern time_t logger_last_activity;

volatile sig_atomic_t w;
static volatile sig_atomic_t t;

static struct sigaction actionTERM;//decalre sigaction structure (for SIGTERM)
static struct sigaction actionINT;//decalre sigaction structure (for SIGTERM)

void terminate (int sigint) //leaved to expand functionality of this app
{
    log_line("Terminating application");
    t = 0;
    w = 0;
}

void destroy_leftovers(void)
{
    log_line("In destroy_leftovers");
    //destroy semaphores and mutex
    log_line("stat_buffer access control destroy");
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&conch);

    log_line("res_buffer access control destroy");
    sem_destroy(&empty_r);
    sem_destroy(&full_r);
    pthread_mutex_destroy(&conch_r);

    log_line("freeing buffer memory");
    //free alocated memory
    ring_buffer_free(stat_buffer);
    ring_buffer_free(res_buffer);
    log_line("freeing analyzer args buffer");
    free(pass);
}

void initialize_program_variables(void)
{
    log_line("Initializing atomic variables");
    w = 1;
    t = 1;
    printf("Enetring initialization process\n");
    
    log_line("Setting a SIGTERM sigaction");
    memset(&actionTERM, 0, sizeof(struct sigaction));//clear the sigaction structure memory
    actionTERM.sa_handler = terminate; //assign a function to this action handler
    sigaction(SIGTERM, &actionTERM, NULL);//assign a signal value to the action structure

    log_line("Setting a SIGINT sigaction");
    memset(&actionINT, 0, sizeof(struct sigaction));//clear the sigaction structure memory
    actionINT.sa_handler = terminate; //assign a function to this action handler
    sigaction(SIGINT, &actionINT, NULL);//assign a signal value to the action structure
    
    //buffers initialization
    printf("Allocating the buffer\n");
    log_line("stat_buffer alocating memory");
    stat_buffer = (ring_buffer*)malloc(sizeof(ring_buffer));//memmory allocation
    if(stat_buffer == NULL)//check if the memmory was alocated
    {
        log_line("stat_buffer was unable to allocate memory. Out of heap memory?");
        perror("NULL exception in malloc\n");
    }
    log_line("Initializing stat_buffer");
    stats_ring_buffer_init(stat_buffer); //initialize stats ring buffer

    log_line("res_buffer alocating memory");
    res_buffer = (ring_buffer*)malloc(sizeof(ring_buffer));//memmory allocation
    if(res_buffer == NULL)//check if the memory was alocated
    {
        log_line("res_buffer was unable to allocate memory. Out of heap memory?");
        perror("NULL exception in malloc\n");
    }
    log_line("Initializing res_buffer");
    res_ring_buffer_init(res_buffer);//initialize results ring buffer

    log_line("pass buffer allocating memory");
    pass = (an_args*)malloc(sizeof(an_args));//allocate memory for args structure (arguments for analyze_stats procedure)
    if(pass == NULL)//check if the memory was alocated
    {
        log_line("pass buffer was unable to allocate memory. Out of heap memory?");
        perror("NULL exception in malloc\n");
    }
    log_line("Initializing pass buffer");
    pass->res_buff = res_buffer;//assign res_buffer pointer to args
    pass->stat_buff = stat_buffer;//assign stat_buffer pointer to args
    //buffers initialization end
    
    log_line("Initilize stat_buffer access control variables");
    //stats_cpu buffer sem and mutex
    sem_init(&empty, 0, BUFFER_SIZE - 1); //initialize the semaphore variable (empty slots in buffer)
    sem_init(&full, 0, 0); //initialize the semaphore variable (full slots in buffer)
    pthread_mutex_init(&conch, NULL); //initialize the mutex

    //results buffer sem and mutex
    log_line("Initialize res_buffer access controle variables");
    sem_init(&empty_r, 0, BUFFER_SIZE - 1); //initialize the semaphore variable (empty slots in buffer)
    sem_init(&full_r, 0, 0); //initialize the semaphore variable (full slots in buffer)
    pthread_mutex_init(&conch_r, NULL); //initialize the mutex
}

/// @brief Reader procedure - starts reading of the /proc/stat file, returns results to stats buffer and oversess access to stats buffer
/// @param buff buffer with stats structure
void reader_procedure(ring_buffer *buff)
{   
    log_line("Starting reader procedure");
    reader_last_activity = time(NULL);
    //thread timeout initialization
    struct timespec r_timeout;
    size_t proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN); //number of phisycial processors 
    //the clock is used to match exactly 1 second window of mesurement
    clock_t start, end; //clock variables
    double cpu_time_used; //time passed in the procedure 
    double one_sec = WINDOW_TIME; //window time in secs
    struct stats_cpu *temp_d = (struct stats_cpu*)malloc(sizeof(struct stats_cpu) * proc_num); //initialize the temp variable for data 
    if(temp_d == NULL)
    {
        log_line("temp_d was unable to allocate memory. Out of heap memory?");
        perror("Could not allocate memory for temporary variable");
    }
    log_line("Entering main reader lopp");
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
            log_line("successfully acquired stat_buffer lock");
            ring_buffer_push(buff, temp_d);//put data into buffer
            pthread_mutex_unlock(&conch);//unlock mutex
        }
        else
        {
            log_line("could not lock mutex on stat_buffer ");
            pthread_mutex_unlock(&conch);
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
    log_line("Starting analyzer procedure");
    analyzer_last_activity = time(NULL);
    usleep(50000);
    //thread timeout init
    struct timespec a_timeout; //special structure for timeout
    //helper variables init
    size_t proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN); //number of phisycial processors 
    size_t var_size = (size_t)(sizeof(struct stats_cpu) * (proc_num)); //size of the aray of stats_cpu structures
    log_line("Allocating memory for results array");
    u_int *results = (u_int *)malloc(sizeof(u_int) * proc_num); //allocate results array
    if(results == NULL)
    {
        perror("Error allocating memory");
        log_line("results could not allocate memory. Out of heap memory?");
    }
    log_line("Allocating memory for prev array");
    struct stats_cpu *prev = (struct stats_cpu *)malloc(var_size); //allocate previous stats array
    if(prev == NULL)
    {
        perror("Error allocating memory");
        log_line("prev could not allocate memory. Out of heap memory?");
    }
    log_line("Allocating memory for curr array");
    struct stats_cpu *curr = (struct stats_cpu *)malloc(var_size); //allocate current stats array
    if(curr == NULL)
    {
        perror("Error allocating memory");
        log_line("curr could not allocate memory. Out of heap memory?");
    }
    log_line("Initializing first values of data to analyze");
    initialize_stats_var(prev, 0);//initialize first array to 0's
    initialize_stats_var(curr, 1);//second is initialized to 1 to avoid undefined behaviour
    clock_t start, end; //clock variables
    double cpu_time_used; //time passed in the procedure 
    double one_sec = WINDOW_TIME; //window time in secs

    //main analyzer thread loop
    log_line("Entering main analyzer loop");
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
            log_line("Succesfully acquired stat_buffer lock");
            ring_buffer_pop(args->stat_buff, curr);//get the data from the stats buffer
            pthread_mutex_unlock(&conch);
        }
        else
        {
            log_line("Could not acquired stat_buffer lock");
            pthread_mutex_unlock(&conch);
            //TODO:Log message that thread timed out while trying to acquire mutex
        }
        sem_post(&empty);

        //computations
        log_line("Going to analyze now...");
        analyzer_compute(prev, curr, results);//analyze stats
        log_line("Data analyzed");
        //results buffer operation
        sem_wait(&empty_r);
        if(pthread_mutex_timedlock(&conch_r, &a_timeout) == 0)
        {
            log_line("Successfully acquired res_buffer lock");
            ring_buffer_push(args->res_buff, results);//put data into results buffer
            pthread_mutex_unlock(&conch_r);
        }
        else
        {
            log_line("Could not acquire res_buffer lock");
            pthread_mutex_unlock(&conch_r);
            //TODO:Log message that thread timed out while trying to acquire mutex
        }
        sem_post(&full_r);

        //variables switching
        log_line("Copying memory from curr to prev variable");
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
    sleep(1);
    printf("Analyzer is finishing its execution \n");
}

/// @brief Printer procedure - prints the values of percentage for each CPU to the console
/// @param buff u_int array type that stores percentage values
void print_stats(ring_buffer *buff)
{
    log_line("Starting printer procedure");
    printer_last_activity = time(NULL);
    sleep(1);
    struct timespec p_timeout;
    size_t proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN); //number of phisycial processors 
    clock_t start, end;
    double cpu_time_used; //time passed in the procedure 
    double one_sec = WINDOW_TIME; //window time in secs
    log_line("Allocating toPrint memory");
    u_int *toPrint = (u_int *)malloc(sizeof(u_int) * proc_num); //allocate results array
    if(toPrint == NULL)
    {
        log_line("toPrint was unnable to allocate memory. Out of heap memory?");
    }
    printf("CPUsageTracker - mesures CPU usage over time\n");
    printf("Mesurement of stats is approximately in 1 sec intervals\n");
    printf("============================================================\n");
    printf("=                                                           \n");
    log_line("Entering printer main loop");
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
            log_line("Succesfully acquired res_buffer lock");
            ring_buffer_pop(buff, toPrint);
            pthread_mutex_unlock(&conch_r);
        }
        else
        {
            log_line("Could not acquire res_buffer lock");
            pthread_mutex_unlock(&conch_r);
        }
        sem_post(&empty_r);
        log_line("Going to print now...");
        if(*toPrint > 1)//print the results (left from debugging but after second thought could be useful)
        {
            print_the_results(toPrint);
        }
        log_line("Results printed.");

        end = clock();//stop the clock
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; //compute the overal time that reading file and pushing to buffer took
        if(cpu_time_used <= one_sec)
        {
            usleep((__useconds_t)((one_sec-cpu_time_used)*100000)); //sleep for the period left to 1 sec
        }
    }
    free(toPrint);
    usleep(150000);//for synced closing of threads
    puts("Printer is closing");

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
    log_line("Starting main loop of the watchdog");
    while(w)
    {
        time_t timestamp = time(NULL);
        if((timestamp - reader_last_activity) > TIMEOUT )
        {
            //this will gently close the application
            terminate(1);
            log_line("Reader thread had a TIMEOUT");
            sleep(2);
            puts("ERROR: TIMEOUT - Application closed due to thread time out.");

        }
        if((timestamp - analyzer_last_activity) > TIMEOUT )
        {
            //this will gently close the application
            log_line("Analyzer thread had a TIMEOUT");
            terminate(1);
            sleep(2);
            puts("ERROR: TIMEOUT - Application closed due to thread time out.");

        }
        if((timestamp - printer_last_activity) > TIMEOUT )
        {
            //this will gently close the application
            log_line("Printer thread had a TIMEOUT");
            terminate(1);
            sleep(2);
            puts("ERROR: TIMEOUT - Application closed due to thread time out.");
        }
        if((timestamp - logger_last_activity) > TIMEOUT )
        {
            //this will gently close the application
            log_line("Logger thread had a TIMEOUT");
            terminate(1);
            sleep(2);
            puts("ERROR: TIMEOUT - Application closed due to thread time out.");
        }
    }
    log_line("Watchdog thread is closing...");
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

