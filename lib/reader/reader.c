#include <stdio.h>
#include <stdlib.h>
#include "reader.h"

int give_lifesign(int life)
{
    printf("Reader push out\n");
    life = life + 2;
    return life;
}