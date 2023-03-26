#pragma once //in other words #ifndef MAIN.H, #define MAIN.H
#include "structures.h"

static sem_t empty, full, empty_r, full_r;
static pthread_mutex_t conch, conch_r;

static size_t proc_num;

static pthread_t reader, analyzer, printer; //thread declaration

static an_args *pass;

//TODO: write implementation comment
void *reader_procedure(ring_buffer *buff);

void *analyze_stats (an_args *args);

void initialize_stats_var (struct stats_cpu *var, u_ll val);
