#include "logger.h"
#include "../../src/main_p.h"

//EXTERNAL VARIABLES
time_t logger_last_activity;
/*******************/

//INTERNAL VARIABLES
static pthread_mutex_t log_conch;
static short was_written;
static FILE *lfp;
static sig_atomic_t l;
static time_t stamp;
/*******************/

/// @brief Initialize logger procedure
void logger_main (void)
{
    while(l)
    {
        //log timestamp for watchdog
        logger_last_activity = time(NULL);
        if(was_written)
        {
            pthread_mutex_lock(&log_conch);     //lock logger conch
            fflush(lfp);                        //flush lfp stream
            pthread_mutex_unlock(&log_conch);   //unlock logger conch
            was_written = 0;                    //set flag that string was "100%" passed to file 
        }
    }
}

/// @brief Initialize logger module
void logger_init(void)
{
    stamp = time(NULL);
    l = 1;
    pthread_mutex_init(&log_conch, NULL);
    was_written = 0;
    if((lfp = fopen(LOG_FILE, "w")) == NULL)
    {
        perror("Could not create Log.txt file.");
    }
}

/// @brief Log a line
/// @param line string to log
void log_line(char *line)
{
    pthread_mutex_lock(&log_conch);
    fprintf(lfp, "Thread ID: %ld", syscall(__NR_gettid));
    fprintf(lfp, " time: %d ", (int)(time(NULL) - stamp));
    fprintf(lfp, "%s \n", line);
    was_written = 1;
    pthread_mutex_unlock(&log_conch);
}

/// @brief Clears the memory allocated by logger thread closes log file
/// @return returns 0 if thread joined
int logger_destroy(void)
{
    printf("Destroying logger \n");
    l = 0;
    pthread_join(logger, NULL);
    fflush(lfp);
    fclose(lfp);
    pthread_mutex_destroy(&log_conch);
    return 0;
}
