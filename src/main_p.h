#pragma once //in other words #ifndef MAIN.H, #define MAIN.H
#include "structures.h"
#include <signal.h>
#define WINDOW_TIME 2
#define TIMEOUT 10
#define TIMEOUT_COUNT 3
#define THREAD_TIMEOUT 15 //this one should be always bigger number than TIMEOUT macro

extern pthread_t reader, analyzer, printer, logger; //threads are external (linked to main - althought they didn't had to but i think it is more readable this way)
extern volatile sig_atomic_t w;

//TODO: write implementation comment
void reader_procedure(ring_buffer *buff);

void analyze_stats (an_args *args);

void print_stats(ring_buffer *buff);

void initialize_stats_var (struct stats_cpu *var, u_ll val);

void* analyze_proc (void *s);

void* read_proc (void *s);

void* print_proc (void *s);

void terminate (int sigint);

void initialize_program_variables(void);

void destroy_leftovers(void);

void watchdog_watch(void);

