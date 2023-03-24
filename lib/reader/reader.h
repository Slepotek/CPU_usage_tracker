#pragma once 
//TODO: give comments
struct stats_cpu {
    unsigned long long cpu_user; //normal processes executing in user mode
    unsigned long long cpu_nice; //niced processes executing in user mode
    unsigned long long cpu_sys; //processes executing in kernel mode
    unsigned long long cpu_idle; //twiddling thumbs
    unsigned long long cpu_iowait; //waiting for I/O to complete
    unsigned long long cpu_hardirq; //time for normal interrupts
    unsigned long long cpu_softirq; //time for soft interrupts
    unsigned long long cpu_steal; //time spent in VM's
    unsigned long long cpu_guest; //time for procces on virtual cpu
    unsigned long long cpu_guest_nice; //time for nice procces on virtual cpu
};
#define STATS_CPU_SIZE (sizeof(struct stats_cpu)) //macro for size of the stats_cpu structure
#define STAT "/proc/stat"
#define TEST 0
/*  
    Read CPU statistics

    IN:
    @data       Structure that stores the data 
    @proc_nr    Variable to store the number of proccessors read from /proc/stat file

    OUT:
    @data       Structure with statistics (only for different processors)
    @proc_nr    Number of processors

    RETURNS:    Nothing
*/
void read_stats(struct stats_cpu *data);

int give_lifesign(int life);