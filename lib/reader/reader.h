#pragma once 
#include "../../src/structures.h"
#define STATS_CPU_SIZE (sizeof(struct stats_cpu)) //macro for size of the stats_cpu structure
#define STAT "/proc/stat" //macro with stats file path
extern int test;

void read_stats(struct stats_cpu* _data);

