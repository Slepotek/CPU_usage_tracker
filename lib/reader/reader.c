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
int test;//global test variable used in test_reader.c 


void read_stats(struct stats_cpu* _data)
{
    FILE *fp = malloc(sizeof(FILE));    //file structure (allocating memory for the structure to prevent sigsegv)
    struct stats_cpu* idata;            //interfacing structure
    int num_proc = 0;                   //number of procesor (first structure is global, next one is cpu0)
    struct stats_cpu sc;                //helper structure
    char line[8192];                    //there is a good chance that this size is equal to two memory pages and it is the most optimal for this task
    //check for test conditions
    if(test == 1)
    {
        if((fp = fopen("/home/marcel/Code/CPU_usage_tracker/build/lib/reader/test_text", "r")) == NULL)
        {
        perror("Program was unable to open the file"); 
        }
    }
    //if this is not a test then read real data
    else
    {
        if((fp = fopen(STAT, "r")) == NULL) //open file and check if it opened
        {
        perror("Program was unable to open the file"); 
        }
    }
    while(fgets(line, sizeof(line), fp) != NULL) //get one line of the file (it doesn't have to be 8192 but usualy is)
    {
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
            fclose(fp);//close the file stream
            break;//break the loop
        }
    }
}
