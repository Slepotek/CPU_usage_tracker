#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "reader.h"



void read_stats(struct stats_cpu* _data)
{
    #ifdef TEST 1
        #undef STAT
        #define STAT "/home/marcel/Code/CPU_usage_tracker/build/lib/reader/test_text" //optimised for cmake auto tests 
    #endif // TEST
    FILE *fp;                           //file structure
    struct stats_cpu* idata = malloc(sizeof(struct stats_cpu) * (unsigned long)sysconf(_SC_NPROCESSORS_ONLN));
    int num_proc = 0;
    struct stats_cpu sc;                //helper structure
    char line[8192];                    //there is a good chance that this size is equal to two memory pages and it is the most optimal for this task
    if((fp = fopen(STAT, "r")) == NULL)
    {
        perror("Program was unable to open the file"); 
    }
    while(fgets(line, sizeof(line), fp) != NULL) //also the error procedure
    {
        if(!strncmp(line, "cpu ", 4)) //the first entry in proc stat has to get 4 char's
        {
            //zeroize data structure
            memset(_data, 0, STATS_CPU_SIZE);

            sscanf(line + 5, "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                &_data->cpu_user,
                &_data->cpu_nice,
                &_data->cpu_sys,
                &_data->cpu_idle,
                &_data->cpu_iowait,
                &_data->cpu_hardirq,
                &_data->cpu_softirq,
                &_data->cpu_steal,
                &_data->cpu_guest,
                &_data->cpu_guest_nice);
        }
        else if(!strncmp(line, "cpu", 3))//the rest of the entries only need 3
        {
            //zeroize sc structure
            memset(&sc, 0, STATS_CPU_SIZE);

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
            idata = _data + num_proc + 1;
            *idata = sc;
        }
        else if(!strncmp(line, "intr", 4))
        {
            fclose(fp);
            break;
        }
    }
}
