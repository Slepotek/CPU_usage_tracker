#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "buffer.h"
#include "../reader/reader.h" 
#define u_long unsigned long

//TODO:write functions descriptions

void ring_buffer_init(ring_buffer* rb)
{
    unsigned long proc_nr = (u_long)sysconf(_SC_NPROCESSORS_ONLN); //i know, a shortcut here, but time is an essence 
    rb->buffer = malloc((sizeof(struct stats_cpu) * (u_long)(proc_nr)) * BUFFER_SIZE); //alocating memmory cus bad things happen when you don't 
    rb->buffer_end = (char*)rb->buffer + ((sizeof(struct stats_cpu) * (u_long)(proc_nr)) * BUFFER_SIZE);//TODO: delete this one if not needed
    rb->capacity = BUFFER_SIZE; //defined length of 4 elements in the buffer
    rb->count = 0;
    rb->size = sizeof(struct stats_cpu) * proc_nr;
    rb->head = rb->buffer;
    rb->tail = rb->buffer;
}

//TODO:write buffer init for structures other than stat_cpu

void ring_buffer_free(ring_buffer* rb)
{
    free(rb);
}

static void* advance_pointer(void* ptr, size_t sz)
{
    return (char*)ptr + sz;
}

void ring_buffer_push(ring_buffer* rb, const void* item)
{
    if(rb->count == rb->capacity)
    {
        memcpy(rb->head, item, rb->size);
        if(rb->head == rb->buffer_end)
        {
            //TODO:Log message that buffer is full
            rb->head = rb->buffer; //move the buffer head to the begining
        }
        else{
            rb->head = advance_pointer(rb->head, rb->size);
        }
    }
    else
    {
        memcpy(rb->head, item, rb->size);//firstly copy the data under the head pointer
        if(rb->head == rb->buffer_end)
        {
            rb->head = rb->buffer; //move the buffer head to the begining
        }
        else{
            rb->head = advance_pointer(rb->head, rb->size);
        }
        ++rb->count; //increment the count variable
    }
}

void ring_buffer_pop(ring_buffer* rb, void* item)
{
    if(rb->count == 0)
    {
        //the buffer is empty 
        //TODO: Log message that buffer is empty
        //TODO: Maybe some indicator that there was no item in the buffer for the caller? 
        return;
    }
    memcpy(item, rb->tail, rb->size);
    if(rb->tail == rb->buffer_end)
    {
        rb->tail = rb->buffer;
    }
    else
    {
        rb->tail = advance_pointer(rb->tail, rb->size);
    }
    --rb->count;
}
