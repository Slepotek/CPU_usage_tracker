#pragma once

#define LOG_FILE "Log.txt"
#define LOG_LINE 1024

void logger_init(void);
void log_line(char line[LOG_LINE]);
void logger_destroy(void);
void logger_main (void);
