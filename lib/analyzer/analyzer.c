#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "analyzer.h"
#include "../../src/structures.h"
#include "../logger/logger.h"

/// @brief compute the usage percentage of every cpu
/// @param data0 previous data
/// @param data1 current data
/// @param results array of results for each cpu
void analyzer_compute(struct stats_cpu *data0, struct stats_cpu *data1, u_int *results)
{
    log_line("Analysis start...");
    int proc_num = (int)sysconf(_SC_NPROCESSORS_ONLN);
    for(int i = 0; i < proc_num; i++)
    {
        /*
        *  Algorithm by Vangelis Tasoulas 
        *  https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
        * 
        */
        u_ll prevIdle = data0->cpu_idle + data0->cpu_iowait;
        u_ll idle = data1->cpu_idle + data1->cpu_iowait; 
        u_ll prevNonIdle = data0->cpu_user + data0->cpu_nice + data0->cpu_sys + data0->cpu_hardirq + data0->cpu_softirq + data0->cpu_steal; //27
        u_ll nonIdle = data1->cpu_user + data1->cpu_nice + data1->cpu_sys + data1->cpu_hardirq + data1->cpu_softirq + data1->cpu_steal; //35
        u_ll prevTotal = prevIdle + prevNonIdle;
        u_ll total = idle + nonIdle;
        //differentiate: actual value minus the previous one
        double totald = (double)(total - prevTotal); 
        double idled = (double)(idle - prevIdle);
        double thePercent = (totald - idled)/totald;

        *results = (u_int)(thePercent * 100); //pass the value to proper index in results array (with type casting)
        if(i == proc_num - 1) //this one prevents unnecessary incrementation of array pointers
        {
            return;
        }
        else{
            results++;//move arrays indexes
            data0++;
            data1++;
        }
    }
    log_line("Analysis end...");
    return;
}
