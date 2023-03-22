#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "reader.h"

int main(int argc, char* argv[])
{
    if(argc > 1){
        int response = give_lifesign(atoi(argv[1]));
        printf("The reader response is %d\n", response);
    }
    else
    {
        printf("No commands\n");
    }

    printf("Now in the main method\n");
    printf("Press any key to close\n");
    getchar();
    return 0;
}