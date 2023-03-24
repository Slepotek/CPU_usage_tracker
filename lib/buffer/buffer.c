#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "buffer.h"
#include "../reader/reader.h" 

void ring_buffer_init(ring_buffer* rb)
{
    u_long proc_nr =  (u_long)sysconf(_SC_NPROCESSORS_ONLN); //i know, a shortcut here, but time is an essence 
    rb->buffer = malloc(sizeof(struct stats_cpu) * (u_long)(proc_nr));
    rb->buffer_end = (char*)rb->buffer + (sizeof(struct stats_cpu) * (u_long)(proc_nr));//TODO: delete this one if not needed
    rb->capacity = 4;
    rb->count = 0;
    rb->size = sizeof(struct stats_cpu) * proc_nr;
    rb->head = rb->buffer;
    rb->tail = rb->buffer;
}

void ring_buffer_free(ring_buffer* rb)
{
    free(rb->buffer);
}

static void* advance_pointer(void* ptr, size_t sz)
{
    return (char*)ptr + sz;
}

void ring_buffer_push(ring_buffer* rb, const void* item)
{
    if(rb->count + 1 == rb->capacity)
    {
        //the buffer is full
        //TODO: try to implement here reder thred stop with mutex
        //TODO: implement the return mechanism returning to the first element of the buffer
        printf("The buffer is full");
    }
    else
    {
        memcpy(rb->head, item, rb->size);//firstly copy the data under the head pointer
        rb->head = advance_pointer(rb->head, rb->size); //move head to new (empty field)
        ++rb->count; //increment the count variable
    }
}

void ring_buffer_pop(ring_buffer* rb, void* item)
{
    if(rb->count == 0)
    {
        //the buffer is empty
        return;
    }
    memcpy(item, rb->tail, rb->size);
    --rb->count;
}
