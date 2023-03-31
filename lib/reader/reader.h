#pragma once 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "../../src/structures.h"                   //ring buffer structures and args (global)

#define STATS_CPU_SIZE (sizeof(struct stats_cpu))   //macro for size of the stats_cpu structure
#define STAT "/proc/stat"                           //macro with stats file path

//EXTERNALS provided by the reader
extern int test;
extern time_t reader_last_activity;

//reader functions
void reader_init (void);

void reader_main (ring_buffer *buff);

void reader_destroy(void);

void read_stats(struct stats_cpu* _data);
void read_stats_test(struct stats_cpu* _data);

