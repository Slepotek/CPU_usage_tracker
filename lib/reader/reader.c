#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "reader.h"
#include "../../src/structures.h"
#include "../logger/logger.h"
int test;//global test variable used in test_reader.c 


void read_stats(struct stats_cpu* _data)
{
    log_line("Start reading /proc/stat file...");
    FILE *fp;    //file structure (allocating memory for the structure to prevent sigsegv)
    struct stats_cpu* idata;            //interfacing structure
    int num_proc = 0;                   //number of procesor (first structure is global, next one is cpu0)
    struct stats_cpu sc;                //helper structure
    char line[8192];                    //there is a good chance that this size is equal to two memory pages and it is the most optimal for this task
    //check for test conditions
    if(test == 1)
    {
        if((fp = fopen("/home/marcel/Code/CPU_usage_tracker/build/lib/reader/test_text", "r")) == NULL)
        {
            perror("Program was unable to open the /proc/stat file"); 
            return;
        }
    }
    //if this is not a test then read real data
    else
    {
        log_line("Open file...");
        if((fp = fopen(STAT, "r")) == NULL) //open file and check if it opened
        {
            log_line("Reader was unable to open /proc/stat file");
            perror("Program was unable to open the /proc/stat file"); 
        }
    }
    while(fgets(line, sizeof(line), fp) != NULL) //get one line of the file (it doesn't have to be 8192 but that is safe amount)
    {
        log_line("Get one line");
        if(!strncmp(line, "cpu ", 4)) //the first entry in proc stat has to get 4 char's
        {
            continue; //skip the global element
        }
        else if(!strncmp(line, "cpu", 3))//the rest of the entries only need 3
        {
            //zeroize sc structure
            memset(&sc, 0, STATS_CPU_SIZE);

            //paste read values to sc helper structure
            sscanf(line + 3, "%d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                &num_proc,
                &sc.cpu_user,
                &sc.cpu_nice,
                &sc.cpu_sys,
                &sc.cpu_idle,
                &sc.cpu_iowait,
                &sc.cpu_hardirq,
                &sc.cpu_softirq,
                &sc.cpu_steal,
                &sc.cpu_guest,
                &sc.cpu_guest_nice);
            idata = _data + num_proc;//give the interface variable address of the appropirate index at _data array (array of stats_cpu)
            *idata = sc;//paste the data of sc at the address pointed by idata
        }
        else if(!strncmp(line, "intr", 4))//if relevant data are passed
        {
            log_line("File was read, closing");
            fclose(fp);//close the file stream
            break;//break the loop
        }
    }
}
