#include "reader.h"
#include "../logger/logger.h"
#include "../buffer/buffer.h"
#include "../watchdog/watchdog.h"
#include "../../src/main_p.h"

//EXTERNAL VARIABLES
int test;
time_t reader_last_activity;    //activity variable for watchdog
extern volatile sig_atomic_t t;          //reader main loop index
extern sem_t empty, full;       //semaphores for the stats_buffer
extern pthread_mutex_t conch;   //mutex for the stats_buffer
/********************/

//INTERNAL VARIABLES
static clock_t start, end;             //clock variables
static double cpu_time_used;           //time passed in the procedure 
static struct timespec r_timeout;      //thread timeout structure
static size_t proc_num;                //number of processors
static double one_sec;                 //data read window time variable
static struct stats_cpu *temp_d;       //temporary variable to store read data
static FILE *fp;                       //file structure (allocating memory for the structure to prevent sigsegv)
static struct stats_cpu* idata;        //interfacing structure
static int num_proc = 0;               //number of procesor (first structure is global, next one is cpu0)
static struct stats_cpu sc;            //helper structure
static char line[8192];                //there is a good chance that this size is equal to two memory pages and it is the most optimal for this task
static char path [FILENAME_MAX];       //test variables used to universalise the path to test_text
static char currentDir[FILENAME_MAX];
/********************/

/// @brief Reads /proc/stat file and puts the data at the _data address
/// @param _data address where to put data read from the /proc/stat file
void read_stats(struct stats_cpu* _data) //function is not static because test_reader.c is using it
{
    log_line("Start reading /proc/stat file...");
    //if this is not a test then read real data

    log_line("Open file...");
    //open file and check if it opened
    if(test)
    {
        getcwd(currentDir, FILENAME_MAX);
        snprintf(path, FILENAME_MAX, "%s/test_text", currentDir);
        if((fp = fopen(path, "r")) == NULL)
        {
            log_line("Reader was unable to open test_text file");
            perror("Program was unable to open the test_text file"); 
            perror(path);
        }
    }
    else
    {
        if((fp = fopen(STAT, "r")) == NULL) 
        {
            log_line("Reader was unable to open /proc/stat file");
            perror("Program was unable to open the /proc/stat file"); 
        }
    }
    //get one line of the file (it doesn't have to be 8192 but that is safe amount)
    while(fgets(line, sizeof(line), fp) != NULL) 
    {
        log_line("Get one line");
        //the first entry in proc stat has to get 4 char's
        if(!strncmp(line, "cpu ", 4)) 
        {
            //skip the global element
            continue; 
        }
        //the rest of the entries only need 3
        else if(!strncmp(line, "cpu", 3))
        {
            //zeroize sc structure
            memset(&sc, 0, STATS_CPU_SIZE);

            //paste read values to sc helper structure
            sscanf(line + 3, "%d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                &num_proc,
                &sc.cpu_user,
                &sc.cpu_nice,
                &sc.cpu_sys,
                &sc.cpu_idle,
                &sc.cpu_iowait,
                &sc.cpu_hardirq,
                &sc.cpu_softirq,
                &sc.cpu_steal,
                &sc.cpu_guest,
                &sc.cpu_guest_nice
            );
            //give the interface variable address of the appropirate index at _data array (array of stats_cpu)
            idata = _data + num_proc;
            //paste the data of sc at the address pointed by idata
            *idata = sc;
        }
        //if no relevant data are passed
        else if(!strncmp(line, "intr", 4))
        {
            log_line("File was read, closing");
            fclose(fp);     //close the file stream
            break;          //break the loop
        }
    }
}
/// @brief Initialize the reader procedure variables
void reader_init (void)
{
    reader_last_activity = time(NULL);                                          //first read out so that watchdog won't fire off too early
    proc_num = (size_t)sysconf(_SC_NPROCESSORS_ONLN);                           //number of phisycial processors 
    one_sec = WINDOW_TIME;                                                      //window time in secs
    temp_d = (struct stats_cpu*)malloc(sizeof(struct stats_cpu) * proc_num);    //initialize the temp variable for data 
    if(temp_d == NULL)
    {
        log_line("temp_d was unable to allocate memory. Out of heap memory?");
        perror("Could not allocate memory for temporary variable");
    }
}

/// @brief Start the main reader loop
/// @param buff the ring buffer to witch /proc/stat data is to be copied
void reader_main (ring_buffer *buff)
{
    log_line("Entering main reader lopp");
    while(t)
    {
        //log the loop start time for watchdog
        reader_last_activity = time(NULL); 
        //get time for timed semaphore and mutex
        clock_gettime(CLOCK_REALTIME, &r_timeout); 
        r_timeout.tv_sec += THREAD_TIMEOUT;
        //the clock is used to match exactly 1 second window of mesurement
        //start the clock count
        start = clock();
        //trigger /proc/stat file reading (no access controll - used only by one thread)
        read_stats(temp_d);
        //set semaphore (decrement number of empty slots in buffer)
        sem_wait(&empty);
        //set mutex 
        if(pthread_mutex_timedlock(&conch, &r_timeout) == 0) 
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
        //set semaphore (increment the number of full buffer slots)
        sem_post(&full);
        //stop the clock
        end = clock();
        //compute the overal time that reading file and pushing to buffer took
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; 
        if(cpu_time_used <= one_sec)
        {
            //sleep for the period left to 1 sec if operations took less than 1 sec
            usleep((__useconds_t)((one_sec-cpu_time_used)*100000)); 
        }
    }

}

/// @brief Clear memory allocated for reader
void reader_destroy(void)
{
    //free alocated memory if the thread stops
    free(temp_d);
    printf("Reader is finishing execution\n");
}
