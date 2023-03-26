#pragma once //in other words #ifndef MAIN.H, #define MAIN.H
#include "structures.h"
#include <signal.h>
#define WINDOW_TIME 2

extern sem_t empty, full, empty_r, full_r;

extern pthread_mutex_t conch, conch_r;

extern pthread_t reader, analyzer, printer; //thread declaration

extern an_args *pass;

//extern volatile int process_end;
extern volatile sig_atomic_t t;

//TODO: write implementation comment
void reader_procedure(ring_buffer *buff);

void analyze_stats (an_args *args);

void print_stats(ring_buffer *buff);

void initialize_stats_var (struct stats_cpu *var, u_ll val);
