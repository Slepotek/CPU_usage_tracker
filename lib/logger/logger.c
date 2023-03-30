#include "logger.h"
#include "../../src/main_p.h"

static pthread_mutex_t log_conch;
static short was_written;
static FILE *lfp;
static sig_atomic_t l;
static time_t stamp;
time_t logger_last_activity;

void logger_main (void)
{
    while(l)
    {
        logger_last_activity = time(NULL);
        if(was_written)
        {
            pthread_mutex_lock(&log_conch);
            fflush(lfp);
            pthread_mutex_unlock(&log_conch);
            was_written = 0;   
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
void log_line(char line[LOG_LINE])
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
