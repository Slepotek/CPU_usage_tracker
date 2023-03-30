#pragma once
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#define TIMEOUT 4
#define THREAD_TIMEOUT 8 //this one should be always bigger number than TIMEOUT macro (test this one)

void watchdog_main (void);
void terminate (int sigint);
