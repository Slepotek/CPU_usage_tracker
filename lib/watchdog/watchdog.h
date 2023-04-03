#pragma once
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#define TIMEOUT 4
#define THREAD_TIMEOUT 8 //in this case it is bigger that timeout, but could be smaller however then app would probably skipp some date from the proc/stat

void watchdog_main (void);
void terminate (int sigint);
