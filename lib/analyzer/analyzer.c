#include "analyzer.h"
#include "../logger/logger.h"
#include "../buffer/buffer.h"
#include "../watchdog/watchdog.h"
#include "../../src/main_p.h"

//EXTERNAL VARIABLES
time_t analyzer_last_activity;
extern volatile sig_atomic_t t;
/******************/

//INTERNAL VARIABLES
static struct timespec a_timeout;          //special structure for timeout
static u_int *results_temp;                //allocate results array
static struct stats_cpu *prev; 
static struct stats_cpu *curr;  
static clock_t start, end;                 //clock variables
static double cpu_time_used;               //time passed in the procedure 
static double one_sec;                     //window time in secs
//helper variables
static size_t proc_num;                    //number of phisycial processors 
static size_t var_size;                    //size of the aray of stats_cpu structures
/******************/

/// @brief compute the usage percentage of every cpu
/// @param data0 previous data
/// @param data1 current data
/// @param results array of results for each cpu
void analyzer_compute(struct stats_cpu *data0, struct stats_cpu *data1, u_int *results)
{
    log_line("Analysis start...");
    for(int i = 0; i < (int)proc_num; i++)
    {
        /*
        *  Algorithm by Vangelis Tasoulas 
        *  https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
        * 
        */
        u_ll prevIdle = data0->cpu_idle + data0->cpu_iowait;
        u_ll idle = data1->cpu_idle + data1->cpu_iowait; 
        u_ll prevNonIdle = data0->cpu_user + data0->cpu_nice + data0->cpu_sys + data0->cpu_hardirq + data0->cpu_softirq + data0->cpu_steal; 
        u_ll nonIdle = data1->cpu_user + data1->cpu_nice + data1->cpu_sys + data1->cpu_hardirq + data1->cpu_softirq + data1->cpu_steal; 
        u_ll prevTotal = prevIdle + prevNonIdle;
        u_ll total = idle + nonIdle;
        //differentiate: actual value minus the previous one
        double totald = (double)(total - prevTotal); 
        double idled = (double)(idle - prevIdle);
        double thePercent = (totald - idled)/totald;
        //pass the value to proper index in results array (with type casting)
        *results = (u_int)(thePercent * 100); 
        //this one prevents unnecessary incrementation of array pointers
        if(i == (int)(proc_num - 1))
        {
            return;
        }
        else{
            //move arrays indexes
            results++;
            data0++;
            data1++;
        }
    }
    log_line("Analysis end...");
    return;
}

/// @brief Initialize analyzer procedure variables
void analyzer_init (void)
{
    analyzer_last_activity = time(NULL);
    usleep(50000);
    proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN);                       //number of phisycial processors 
    var_size = (size_t)(sizeof(struct stats_cpu) * (proc_num));             //size of the aray of stats_cpu structures
    log_line("Allocating memory for results array");
    results_temp = (u_int *)malloc(sizeof(u_int) * proc_num);                   //allocate results array
    if(results_temp == NULL)
    {
        perror("Error allocating memory");
        log_line("results could not allocate memory. Out of heap memory?");
    }
    log_line("Allocating memory for prev array");
    prev = (struct stats_cpu *)malloc(var_size);                            //allocate previous stats array
    if(prev == NULL)
    {
        perror("Error allocating memory");
        log_line("prev could not allocate memory. Out of heap memory?");
    }
    log_line("Allocating memory for curr array");
    curr = (struct stats_cpu *)malloc(var_size);                            //allocate current stats array
    if(curr == NULL)
    {
        perror("Error allocating memory");
        log_line("curr could not allocate memory. Out of heap memory?");
    }
    log_line("Initializing first values of data to analyze");
    initialize_stats_var(prev, 0);                                          //initialize first array to 0's
    initialize_stats_var(curr, 1);                                          //second is initialized to 1 to avoid undefined behaviour 
    one_sec = WINDOW_TIME;                                                  //set the time window for processing the data
}

/// @brief Main procedure of analyzer thread
/// @param stats /proc/stat data buffer pointer
/// @param res computations results buffer pointer
void analyzer_main (ring_buffer *stats, ring_buffer *res)
{
    log_line("Entering main analyzer loop");
    while (t)
    {
        analyzer_last_activity = time(NULL);                                //set the first timestamp fr watchdog so that it would not fire too early
        clock_gettime(CLOCK_REALTIME, &a_timeout);                          //acquire current time
        a_timeout.tv_sec += THREAD_TIMEOUT;                                 //add thread timeout time to current time
        start = clock();                                                    //start the clock
        
        //STATS BUFFER OPERATIONS
        sem_wait(&full);                                                    //set semaphore (decrement number of full slots in stats_buffer)
        if(pthread_mutex_timedlock(&conch, &a_timeout) == 0)
        {
            log_line("Succesfully acquired stat_buffer lock");
            ring_buffer_pop(stats, curr);                                   //get the data from the stats buffer
            pthread_mutex_unlock(&conch);
        }
        else
        {
            log_line("Could not acquired stat_buffer lock");
            pthread_mutex_unlock(&conch);
        }
        sem_post(&empty);

        //COMPUTATIONS
        log_line("Going to analyze now...");
        analyzer_compute(prev, curr, results_temp);                         //analyze stats
        log_line("Data analyzed");
        
        //RESULTS BUFFER OPERATIONS
        sem_wait(&empty_r);
        if(pthread_mutex_timedlock(&conch_r, &a_timeout) == 0)
        {
            log_line("Successfully acquired res_buffer lock");
            ring_buffer_push(res, results_temp);                            //put data into results buffer
            pthread_mutex_unlock(&conch_r);
        }
        else
        {
            log_line("Could not acquire res_buffer lock");
            pthread_mutex_unlock(&conch_r);
        }
        sem_post(&full_r);

        //VARIABLES SWITCHING
        log_line("Copying memory from curr to prev variable");
        memcpy(prev, curr, var_size);

        end = clock();                                                      //stop the clock
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;          //compute the overal time that reading file and pushing to buffer took
        if(cpu_time_used <= one_sec)
        {
            usleep((__useconds_t)((one_sec-cpu_time_used)*100000));         //sleep for the period left to 1 sec
        }
    }
}

/// @brief Clean memory used by analyzer procedure
void analyzer_destroy (void)
{
    //free allocated memory 
    free(results_temp);
    free(prev);
    free(curr);
    sleep(1);
    printf("Analyzer is finishing its execution \n");
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
