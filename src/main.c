//This app only counts physical processors, if on VM then only VM processors
//It is not compatible with processor hot plugin, althought not likely the possibility always exists. 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "main_p.h"
#include "main.h"
#include "../lib/logger/logger.h"

extern pthread_t reader, analyzer, printer, logger, watchdog; //thread declaration
extern volatile sig_atomic_t w;

int main(void)
{
    system("clear");
    logger_init(); //initialize logger
    pthread_create(&logger, NULL, logger_proc, NULL);
    log_line("Initializing program components");
    initialize_program_variables();
    log_line("Components initialized ");

    log_line("Starting threads");
    pthread_create(&reader, NULL, read_proc , NULL); //run the reader thread (the warning about bad conversion type can be overcomed by creating explicit function calling the base function - redundant code)
    log_line("Reader thread started");
    pthread_create(&analyzer, NULL, analyze_proc, NULL);//run analyzer thread
    log_line("Analyzer thread started");
    pthread_create(&printer, NULL, print_proc, NULL);//run print thread
    log_line("Watchdog thread started");
    pthread_create(&watchdog, NULL, watchdog_proc, NULL);

    log_line("Going into main loop");
    //wait for the thread to finish
    while(w)
    {
        continue;
    }
    log_line("Joining threads");
    pthread_join(reader, NULL);
    log_line("Reader thread closed");
    pthread_join(analyzer, NULL);
    log_line("Analyzer thread closed");
    pthread_join(printer, NULL);
    log_line("Printer thread closed");
    pthread_join(watchdog, NULL);
    log_line("Watchdog thread closed");
    
    printf("Gentle closing finished \n");

    log_line("Destroing variables");
    destroy_leftovers();
    log_line("Variables destroyed");

    logger_destroy();
    pthread_join(logger, NULL);
    printf("Logger closed\n");
    printf("Resources freed\n");
    printf("Closed.\n");
    return 0;
}

void* watchdog_proc (void *s)
{
    watchdog_watch();
    return s;
}

void* logger_proc(void *s)
{
    logger_main();
    return s;
}
