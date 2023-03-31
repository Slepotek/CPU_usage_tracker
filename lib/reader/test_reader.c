#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "reader.h"
#include "../../src/structures.h"
#include "../logger/logger.h"
extern int test;

int main (void)
{
    test = 1;
    logger_init();
    printf("Running a reader test\n");
    struct stats_cpu *test_var;
    test_var = (struct stats_cpu *)malloc(sizeof(struct stats_cpu) * (u_long)sysconf(_SC_NPROCESSORS_ONLN));
    test = 1;
    read_stats(test_var);
    logger_destroy();
    //Test 1
    int user = (int)test_var->cpu_user;
    assert(user == 25982);
    printf("Test 1 passed\n");
    //Test 2
    test_var = test_var + 1;
    user = (int)test_var->cpu_nice;
    assert(user == 28);
    printf("Test 2 passed\n");
    //Test 3
    test_var = test_var + 1;
    user = (int)test_var->cpu_sys;
    assert(user == 12442);
    printf("Test 3 passed\n");
    //Test 4
    test_var = test_var + 1;
    user = (int)test_var->cpu_idle;
    assert(user == 237762);
    printf("Test 4 passed\n");
}
