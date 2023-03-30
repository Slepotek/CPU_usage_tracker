#include "printer.h"
#include "../logger/logger.h"
#include "../buffer/buffer.h"
#include "../watchdog/watchdog.h"
#include "../../src/main_p.h"


//EXTERNAL VARIABLES
time_t printer_last_activity;
extern volatile sig_atomic_t t;
/*******************/

//INTERNAL VARIABLES
static short sign;
static struct timespec p_timeout;
static size_t proc_num;
static clock_t start, end;
static double cpu_time_used; //time passed in the procedure 
static double one_sec; //window time in secs
static u_int *toPrint;
/*******************/

/// @brief Function prints in a specified formating results aquired from the results buffer 
/// @param printArr pointer to array of values to print
static void print_the_results (u_int *printArr)
{
    log_line("Printing the results");
    sign = STAT_BAR_SIGN;
    //TODO:diferentiate between termux and other terminals
    //escape control sequence for TERMUX
    printf("\e[7;1H\e[J");
    for (int i = 0; i < (int)proc_num; i++)
    {
        short percent = (short)*printArr;
        printf("= CPU%-1d load: %-3d ", i, percent);
        for(int j = 0; j <= percent; j += 10)
        {
            printf("%c", sign);
        }
        printf("\n");
    }
    printf("=                                                           \n");
    printf("============================================================\n");
    fprintf(stdout, "Press CTRL + C to close: \n");
}


/// @brief Initialization of printer procedure
void printer_init (void)
{
    printer_last_activity = time(NULL);                         //first timestamp so that watchdog would not fire too early
    sleep(1);
    proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN);           //number of phisycial processors 
    one_sec = WINDOW_TIME;                                      //window time in secs
    log_line("Allocating toPrint memory");
    toPrint = (u_int *)malloc(sizeof(u_int) * proc_num);        //allocate memory for results array
    if(toPrint == NULL)
    {
        perror("Could not allocate memory for array");
        log_line("toPrint was unnable to allocate memory. Out of heap memory?");
    }
    printf("CPUsageTracker - mesures CPU usage over time\n");
    printf("Mesurement of stats is approximately in 1 sec intervals\n");
    printf("============================================================\n");
    printf("=                                                           \n");
    log_line("Entering printer main loop");
}

/// @brief Main loop of printer procedure
/// @param res pointer to results buffer
void printer_main (ring_buffer *res)
{
    while(t)
    {
        //log the loop start time for watchdog
        printer_last_activity = time(NULL);
        //get time for timed semaphore and mutex
        clock_gettime(CLOCK_REALTIME, &p_timeout);
        p_timeout.tv_sec += THREAD_TIMEOUT;
        //the clock
        start = clock();
        //read from results buffer
        //set semaphore (decrement number of full slots in buffer)
        sem_wait(&full_r);
        //set mutex
        if(pthread_mutex_timedlock(&conch_r, &p_timeout) == 0)
        {
            log_line("Succesfully acquired res_buffer lock");
            ring_buffer_pop(res, toPrint);//put data into buffer
            pthread_mutex_unlock(&conch_r);//unlock mutex
        }
        else
        {
            log_line("Could not acquire res_buffer lock");
            pthread_mutex_unlock(&conch_r);
        }
        //set semaphore (increment the number of full buffer slots)
        sem_post(&empty_r);
        log_line("Going to print now...");
        if(*toPrint > 1)//print the results (proved to be usefull, suspends ambigous results)
        {
            print_the_results(toPrint); //print the results to the standard output
        }
        log_line("Results printed.");
        //stop the clock
        end = clock();
        //compute the overal time that reading file and pushing to buffer took
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; 
        if(cpu_time_used <= one_sec)
        {
            //sleep for the period left to 1 sec
            usleep((__useconds_t)((one_sec-cpu_time_used)*100000)); 
        }
    }
}

/// @brief Clear memory allocated for printer
void printer_destroy(void)
{
    //free memory allocated for printer
    free(toPrint);
    //for synced closing of threads
    usleep(150000);
    puts("Printer is closing");
}

/// @brief Function to return proper escape sequence to given terminal
/// @return proper escape sequence matched to the terminal
static const char* clear_screen_sequence(void)
{
    // Get the value of the TERM environment variable
    const char *term = getenv("TERM");
    if (term == NULL) {
        // The TERM environment variable is not set
        perror("Error: TERM environment variable is not set\n");
    }
    
    // Return the appropriate escape sequence based on the terminal type
    if (strcmp(term, "xterm") == 0) {
        // Xterm terminal
        return "\033[7;1H\033[J";
    } else if (strcmp(term, "vt100") == 0) {
        // VT100 terminal
        return "\033[7;1H\033[J";
    } else {
        // Unknown terminal type
        fprintf(stderr, "Error: unknown terminal type: %s\n", term);
        exit(1);
    }
}
