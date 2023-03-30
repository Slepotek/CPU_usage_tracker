#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "buffer.h"
#include "../../src/structures.h"
#include "../logger/logger.h"

//TODO:write functions descriptions

/// @brief Initializes ring_buffer structure to store stats_cpu arrays
/// @param rb pointer to given ring_buffer
void stats_ring_buffer_init(ring_buffer* rb)
{
    unsigned long proc_nr = (u_long)sysconf(_SC_NPROCESSORS_ONLN); //i know, a shortcut here, but time is an essence 
    rb->buffer = malloc((sizeof(struct stats_cpu) * (u_long)(proc_nr)) * BUFFER_SIZE); //alocating memmory (array * number_of_procesors) * size_of_the_buffer
    if(rb->buffer == NULL)
    {
        perror("Error allocatin memory for the buffer");
        log_line("Could not allocate memory for the stats_buffer. Out of heap memory?");
    }
    rb->buffer_end = (char*)rb->buffer + ((sizeof(struct stats_cpu) * (u_long)(proc_nr)) * BUFFER_SIZE);//last address of the buffer
    rb->capacity = BUFFER_SIZE; //defined length of 4 elements in the buffer (can be more - set it in heder file)
    rb->count = 0; //number of elements currently residing in the buffer
    rb->size = sizeof(struct stats_cpu) * proc_nr; //size of unary structure in the buffer (in this case should be an stats_cpu structure)
    rb->head = rb->buffer;
    rb->tail = rb->buffer;
}

/// @brief Initilizes ring_buffer structure to store u_int arrays
/// @param rb pointer to given ring_buffer
void res_ring_buffer_init(ring_buffer* rb)
{
    unsigned long proc_nr = (u_long)sysconf(_SC_NPROCESSORS_ONLN); //i know, a shortcut here, but time is an essence 
    rb->buffer = malloc((sizeof(struct stats_cpu) * (u_long)(proc_nr)) * BUFFER_SIZE); //alocating memmory (array * number_of_procesors) * size_of_the_buffer
    if(rb->buffer == NULL)
    {
        perror("Error allocatin memory for the buffer");
        log_line("Could not allocate memory for the res_buffer. Out of heap memory?");
    }
    rb->buffer_end = (char*)rb->buffer + ((sizeof(u_int) * (u_long)(proc_nr)) * BUFFER_SIZE);//last address of the buffer
    rb->capacity = BUFFER_SIZE; //defined length of 4 elements in the buffer (can be more - set it in heder file)
    rb->count = 0; //number of elements currently residing in the buffer
    rb->size = sizeof(u_int) * proc_nr;//size of unary structure in the buffer (in this case should be an array of u_int)
    rb->head = rb->buffer;
    rb->tail = rb->buffer;
}

/// @brief Free the memory of ring_buffer
/// @param rb pointer to given ring_buffer
void ring_buffer_free(ring_buffer* rb)
{
    log_line("Freeing buffer ");
    free(rb->buffer);
    free(rb);
    log_line("Buffer freed");
}

/// @brief Internal ring buffer structure function to move pointer to next address
/// @param ptr pointer to given addres in ring_buffer structure
/// @param sz size of the element in the buffer
/// @return returns a new address for given pointer
static void* advance_pointer(void* ptr, size_t sz)
{
    return (char*)ptr + sz;
}

/// @brief Push data into ring_buffer structure
/// @param rb given ring_buffer pointer
/// @param item pointer to the data that is to be inserted into ring_buffer
void ring_buffer_push(ring_buffer* rb, const void* item)
{
    if(rb->head == (char*)rb->buffer_end - rb->size) //if head of the buffer hit penultimate element
    {
        memcpy(rb->head, item, rb->size);//firstly copy the data to the head address
        rb->head = rb->buffer; //move the buffer head to the begining of the buffer
        //buffer is full 
        log_line("Buffer is full"); 
    }
    else //if not
    {
        memcpy(rb->head, item, rb->size);//firstly copy the data under the head pointer
        rb->head = advance_pointer(rb->head, rb->size);// then move address one structure forward
    }
    ++rb->count;//increment the element count in the buffer (it never comes to 4 - althought that is the given capacity but this is to aassure memory sefty)
}

void ring_buffer_pop(ring_buffer* rb, void* item)
{
    if(rb->count == 0) //if no elements in the buffer 
    {
        //the buffer is empty 
        log_line("Buffer is empty ");
        return;//simply return
    }
    //if there are elements in the buffer
    memcpy(item, rb->tail, rb->size); //firstly copy the memory
    if(rb->tail == (char*)rb->buffer_end - rb->size) //check if the tail is at the penultimate element
    {
        //if yes
        rb->tail = rb->buffer;//then return to the begining
    }
    else//if not
    {
        rb->tail = advance_pointer(rb->tail, rb->size);//then move address one structure forward
    }
    --rb->count; //decrement the element count in the buffer
}
