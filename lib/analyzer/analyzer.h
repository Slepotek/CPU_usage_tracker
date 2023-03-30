#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../src/structures.h"

extern time_t analyzer_last_activity;

void analyzer_init (void);

void analyzer_main (ring_buffer *stats, ring_buffer *res);

void analyzer_destroy (void);

void initialize_stats_var (struct stats_cpu *var, u_ll val);

void analyzer_compute(struct stats_cpu *data0, struct stats_cpu *data1, u_int *results);
