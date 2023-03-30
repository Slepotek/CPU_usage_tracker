#pragma once

#define LOG_FILE "Log.txt"
#define LOG_LINE 1024

#include "../../src/main_p.h"
#define READER_ID reader
#define ANALYZER_ID analyzer
#define PRINTER_ID printer
#define LOGGER_ID logger //not used in thi implementation
#define WATCHDOG_ID watchdog

extern time_t logger_last_activity;

void logger_init(void);
void log_line(char line[LOG_LINE]);
void logger_destroy(void);
void logger_main (void);
