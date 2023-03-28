#pragma once 

//this header file is shared between all components (could be in shared folder)

#define u_int unsigned int
#define u_ll unsigned long long
#define u_long unsigned long

typedef struct ring_buffer {
    void* buffer; //the data
    void* buffer_end; //the last element
    size_t capacity; //the maximum number of elements in the buffer
    size_t count; //current count of elements in the buffer
    size_t size; //size of the data in the buffer (cpu_stat)
    void* head; //write
    void* tail; //read
} ring_buffer;

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

typedef struct an_args {
    ring_buffer *stat_buff;
    ring_buffer *res_buff;
} an_args;
