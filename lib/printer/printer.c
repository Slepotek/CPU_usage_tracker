#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "printer.h"
#include "../logger/logger.h"


void print_the_results (u_int *printArr)
{
    log_line("Printing the results");
    int proc_num = (int)sysconf(_SC_NPROCESSORS_ONLN);
    short sign = 35;
    printf("\e[7;0H\e[0J");//escape control sequence
    for (int i = 0; i < proc_num; i++)
    {
        short percent = (short)*printArr;
        printf("= CPU%-1d load: %-3d ", i, percent);
        for(int j = 0; j <= percent; j += 10)
        {
            printf("%c", sign);
        }
        printf("\n");
    }
    printf("=                                                           \n");
    printf("============================================================\n");
    fprintf(stdout, "Press CTRL + C to close: \n");
}
