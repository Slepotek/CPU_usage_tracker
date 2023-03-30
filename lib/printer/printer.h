#pragma once 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "../../src/structures.h"

#define STAT_BAR_SIGN 35

extern time_t printer_last_activity;

void printer_init (void);

void printer_main (ring_buffer *res);

void printer_destroy(void);

