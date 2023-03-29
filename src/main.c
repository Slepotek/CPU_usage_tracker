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
    //logger_init(); //initialize logger
    //pthread_create(&logger, NULL, logger_proc, NULL);
    //log_line("Initializing program components");
    initialize_program_variables();

    pthread_create(&reader, NULL, read_proc , NULL); //run the reader thread (the warning about bad conversion type can be overcomed by creating explicit function calling the base function - redundant code)
    pthread_create(&analyzer, NULL, analyze_proc, NULL);//run analyzer thread
    pthread_create(&printer, NULL, print_proc, NULL);//run print thread
    pthread_create(&watchdog, NULL, watchdog_proc, NULL);

    //wait for the thread to finish
    while(w)
    {
        continue;
    }
    pthread_join(reader, NULL);
    pthread_join(analyzer, NULL);
    pthread_join(printer, NULL);
    pthread_join(watchdog, NULL);
    
    printf("Gentle closing finished \n");

    destroy_leftovers();
    //logger_destroy();
    //pthread_join(logger, NULL);
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
