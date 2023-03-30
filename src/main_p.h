#pragma once //in other words #ifndef MAIN_P.H, #define MAIN_P.H
#include "structures.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

//EXTERNALS provided by the main_p
extern sem_t empty, full, empty_r, full_r;
extern pthread_mutex_t conch, conch_r;
extern pthread_t reader, analyzer, printer, logger, watchdog; //thread descriptors
extern volatile sig_atomic_t w;
extern volatile sig_atomic_t t;

void* reader_procedure(void *s);

void* analyze_stats (void *s);

void* print_stats(void *s);

void* watchdog_watch(void*);

void initialize_program_variables(void);

void destroy_leftovers(void);


