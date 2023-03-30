#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "logger.h"


static pthread_mutex_t log_conch;
static char log_buffer[LOG_LINE];
static FILE *lfp;
static sig_atomic_t l;
static time_t stamp;

void logger_main (void)
{
    while(l)
    {
        sleep(1);
        fflush(lfp);
    }
}

/// @brief Initialize logger module
void logger_init(void)
{
    stamp = time(NULL);
    l = 1;
    pthread_mutex_init(&log_conch, NULL);
    memset(&log_buffer, 0, LOG_LINE);
    if((lfp = fopen(LOG_FILE, "w")) == NULL)
    {
        perror("Could not create Log.txt file.");
    }
}

/// @brief Log a line
/// @param line string to log
void log_line(char line[LOG_LINE])
{

    fprintf(lfp, "%d ", (int)(time(NULL) - stamp));
    pthread_mutex_lock(&log_conch);
    fprintf(lfp, "%s \n", line);
    pthread_mutex_unlock(&log_conch);
}

/// @brief Clean up after logger
void logger_destroy(void)
{
    printf("Destroying logger \n");
    fclose(lfp);
    pthread_mutex_destroy(&log_conch);
    l = 0;
}
