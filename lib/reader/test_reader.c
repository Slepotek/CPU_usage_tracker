#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "reader.h"


int main (void)
{
    #undef TEST
    #define TEST 1
    printf("Running a reader test\n");
    struct stats_cpu *test_var = malloc(sizeof(struct stats_cpu) * (unsigned long)sysconf(_SC_NPROCESSORS_ONLN));
    read_stats(test_var);
    //Test 1
    int user = (int)test_var->cpu_user;
    assert(user == 93034);
    printf("Test 1 passed\n");
    //Test 2
    test_var = test_var + 1;
    user = (int)test_var->cpu_nice;
    assert(user == 96);
    printf("Test 2 passed\n");
    //Test 3
    test_var = test_var + 1;
    user = (int)test_var->cpu_sys;
    assert(user == 11464);
    printf("Test 3 passed\n");
    //Test 4
    test_var = test_var + 1;
    user = (int)test_var->cpu_idle;
    assert(user == 239701);
    printf("Test 4 passed\n");
    //Test 5
    test_var = test_var + 1;
    user = (int)test_var->cpu_iowait;
    assert(user == 530);
    printf("Test 5 passed\n");
    printf("All tests passed, good to go.");
}