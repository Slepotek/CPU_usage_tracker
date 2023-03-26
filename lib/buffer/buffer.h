#pragma once

#include "../../src/structures.h"

#define BUFFER_SIZE 10

void stats_ring_buffer_init(ring_buffer* rb);

void res_ring_buffer_init(ring_buffer* rb);

void ring_buffer_free(ring_buffer* rb);

static void* advance_pointer(void* ptr, size_t sz);

void ring_buffer_push(ring_buffer* rb, const void* item);

void ring_buffer_pop(ring_buffer* rb, void* item);

