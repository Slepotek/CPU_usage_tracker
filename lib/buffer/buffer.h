#pragma once

#define BUFFER_SIZE 3

typedef struct ring_buffer {
    void* buffer; //the data
    void* buffer_end; //the last element
    size_t capacity; //the maximum number of elements in the buffer
    size_t count; //current count of elements in the buffer
    size_t size; //size of the data in the buffer (cpu_stat)
    void* head; //write
    void* tail; //read
} ring_buffer;

void ring_buffer_init(ring_buffer* rb);

void ring_buffer_free(ring_buffer* rb);

static void* advance_pointer(void* ptr, size_t sz);

void ring_buffer_push(ring_buffer* rb, const void* item);

void ring_buffer_pop(ring_buffer* rb, void* item);

