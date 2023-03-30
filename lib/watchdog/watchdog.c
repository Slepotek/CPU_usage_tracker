#include "watchdog.h"
#include "../logger/logger.h"

extern time_t logger_last_activity, reader_last_activity, printer_last_activity, analyzer_last_activity;
extern volatile sig_atomic_t w;
extern volatile sig_atomic_t t;

void watchdog_main (void)
{
    sleep(2);
    while(w)
    {
        time_t timestamp = time(NULL);
        if((timestamp - reader_last_activity) > TIMEOUT )
        {
            //this will gently close the application
            log_line("Reader thread had a TIMEOUT");
            //terminate(1);
            sleep(2);
            puts("ERROR: TIMEOUT - Application closed due to thread time out.");

        }
        if((timestamp - analyzer_last_activity) > TIMEOUT )
        {
            //this will gently close the application
            log_line("Analyzer thread had a TIMEOUT");
            //terminate(1);
            sleep(2);
            puts("ERROR: TIMEOUT - Application closed due to thread time out.");

        }
        if((timestamp - printer_last_activity) > TIMEOUT )
        {
            //this will gently close the application
            log_line("Printer thread had a TIMEOUT");
            //terminate(1);
            sleep(2);
            puts("ERROR: TIMEOUT - Application closed due to thread time out.");
        }
        if((timestamp - logger_last_activity) > TIMEOUT )
        {
            //this will gently close the application
            log_line("Logger thread had a TIMEOUT");
            //terminate(1);
            sleep(2);
            puts("ERROR: TIMEOUT - Application closed due to thread time out.");
        }
    }
    log_line("Watchdog thread is closing...");
}

void terminate (int sigint) //leaved to expand functionality of this app
{
    log_line("Terminating application");
    t = 0;
    w = 0;
}
