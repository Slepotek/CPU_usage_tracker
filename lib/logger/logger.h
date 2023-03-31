#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <syscall.h>

#define LOG_FILE "Log.txt"
#define LOG_LINE 1024

#define READER_ID reader
#define ANALYZER_ID analyzer
#define PRINTER_ID printer
#define LOGGER_ID logger //not used in thi implementation
#define WATCHDOG_ID watchdog

extern time_t logger_last_activity;

void logger_init(void);
void log_line(char *line);
int logger_destroy(void);
void logger_main (void);
