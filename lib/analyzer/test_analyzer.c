#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "analyzer.h"
#include "../../src/structures.h"
#include "../logger/logger.h"


int main (void)
{
    int proc_num = 4; //fixed size proc number
    size_t var_size = sizeof(struct stats_cpu) * (u_int)proc_num;
    struct stats_cpu *test_var = malloc(var_size);
    struct stats_cpu *test_var0 = malloc(var_size);
    struct stats_cpu *test_var1 = malloc(var_size);
    struct stats_cpu *test_var2 = malloc(var_size);
    struct stats_cpu *itest_var;

    itest_var = test_var0;
    for(int i = 1; i <= proc_num; i++)
    {
        itest_var->cpu_user = 1000;
        itest_var->cpu_nice = 2000;
        itest_var->cpu_sys = 3000;
        itest_var->cpu_idle = 4000;
        itest_var->cpu_iowait = 5000;
        itest_var->cpu_hardirq = 6000;
        itest_var->cpu_softirq = 7000;
        itest_var->cpu_steal = 8000;
        itest_var->cpu_guest = 9000;
        itest_var->cpu_guest_nice = 1010;

        itest_var = test_var0 + i;
    }
    itest_var = test_var1;
    for(int i = 1; i <= proc_num; i++)
    {
        itest_var->cpu_user = 9000;
        itest_var->cpu_nice = 8000;
        itest_var->cpu_sys = 7000;
        itest_var->cpu_idle = 6000;
        itest_var->cpu_iowait = 5000;
        itest_var->cpu_hardirq = 4000;
        itest_var->cpu_softirq = 3000;
        itest_var->cpu_steal = 2000;
        itest_var->cpu_guest = 1000;
        itest_var->cpu_guest_nice = 1010;

        itest_var = test_var1 + i;
    }
    itest_var = test_var;
    for(int i = 1; i <= proc_num; i++)
    {
        itest_var->cpu_user = 0;
        itest_var->cpu_nice = 0;
        itest_var->cpu_sys = 0;
        itest_var->cpu_idle = 0;
        itest_var->cpu_iowait = 0;
        itest_var->cpu_hardirq = 0;
        itest_var->cpu_softirq = 0;
        itest_var->cpu_steal = 0;
        itest_var->cpu_guest = 0;
        itest_var->cpu_guest_nice = 0;

        itest_var = test_var + i;
    }
    itest_var = test_var2;
    for(int i = 1; i <= proc_num; i++)
    {
        itest_var->cpu_user = 1;
        itest_var->cpu_nice = 1;
        itest_var->cpu_sys = 1;
        itest_var->cpu_idle = 1;
        itest_var->cpu_iowait = 1;
        itest_var->cpu_hardirq = 1;
        itest_var->cpu_softirq = 1;
        itest_var->cpu_steal = 1;
        itest_var->cpu_guest = 1;
        itest_var->cpu_guest_nice = 1;

        itest_var = test_var2 + i;
    }
    
    u_int results[proc_num];

    logger_init();
    analyzer_init();
    analyzer_compute(test_var, test_var2, &results[0]);
    for(int i = 0; i < proc_num; i++)
    {
        assert(results[i] == 75);
    }
    analyzer_compute(test_var0, test_var1, &results[0]);
    analyzer_destroy();
    logger_destroy();
    /*
     *  Algorithm by Vangelis Tasoulas 2014
     *  https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
     * 
    */
    u_ll prevIdle = test_var0->cpu_idle + test_var0->cpu_iowait;
    u_ll idle = test_var1->cpu_idle + test_var1->cpu_iowait;
    u_ll prevNonIdle = test_var0->cpu_user + test_var0->cpu_nice + test_var0->cpu_sys + test_var0->cpu_hardirq + test_var0->cpu_softirq + test_var0->cpu_steal;
    u_ll nonIdle = test_var1->cpu_user + test_var1->cpu_nice + test_var1->cpu_sys + test_var1->cpu_hardirq + test_var1->cpu_softirq + test_var1->cpu_steal;
    u_ll prevTotal = prevIdle + prevNonIdle;
    u_ll total = idle + nonIdle;
    //differentiate: actual value minus the previous one
    double totald = (double)(total - prevTotal); 
    double idled = (double)(idle - prevIdle);
    double thePercent = (totald - idled)/totald;
    
    for(int i = 0; i < proc_num; i++)
    {
        assert(results[i] == (u_int)(thePercent*100));
    }
    printf("Analyzer test succeded\n");
    free(test_var);
    free(test_var0);
    free(test_var1);
    free(test_var2);
    return 0;
}
