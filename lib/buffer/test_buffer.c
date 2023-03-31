#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <assert.h>
#include "buffer.h"
#include "test_buffer.h"
#include "../../src/structures.h"
#include "../logger/logger.h"

static sem_t empty, full;
static pthread_mutex_t conch;
static pthread_t producer, consumer;
static struct stats_cpu *test_var;
struct stats_cpu *istat;

int main (void)
{
    logger_init();
    //TEST Initialization
    size_t m_buff_size = sizeof(ring_buffer);
    ring_buffer *rb = malloc(m_buff_size);
    test_var = malloc(sizeof(struct stats_cpu));
    stats_ring_buffer_init(rb);
    
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&conch, NULL);

    test_var->cpu_user = 111;
    test_var->cpu_nice = 222;
    test_var->cpu_sys = 333;
    test_var->cpu_idle = 444;
    test_var->cpu_iowait = 555;
    test_var->cpu_hardirq = 666;
    test_var->cpu_softirq = 777;
    test_var->cpu_steal = 888;
    test_var->cpu_guest = 999;
    test_var->cpu_guest_nice = 1010;

    pthread_create(&producer, NULL, &producerSeq, rb);
    pthread_create(&consumer, NULL, &consumerSeq, rb);

    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&conch);

    logger_destroy();
    //ring_buffer_free(rb);
    free(test_var);
    printf("Ring buffer test succesfull\n");
}

void* producerSeq (ring_buffer *rb)
{
    for(int i = 0; i <= 100; i++)
    {
        sem_wait(&empty);
        pthread_mutex_lock(&conch);
        ring_buffer_push(rb, test_var);
        pthread_mutex_unlock(&conch);
        sem_post(&full);
    }
    printf("Finished adding structures to buffer\n");
    return 0;
}

void* consumerSeq (ring_buffer *rb)
{
    istat = malloc(sizeof(struct stats_cpu));
    for(int i = 0; i <= 100; i++)
    {
        sem_wait(&full);
        pthread_mutex_lock(&conch);
        ring_buffer_pop(rb, istat);
        pthread_mutex_unlock(&conch);
        sem_post(&empty);
        assert(istat->cpu_user == test_var->cpu_user);
        assert(istat->cpu_sys == test_var->cpu_sys);
        assert(istat->cpu_hardirq == test_var->cpu_hardirq);
        assert(istat->cpu_guest == test_var->cpu_guest);
        memset(istat, 0, sizeof(struct stats_cpu));
    }
    printf("Finished reading structures from buffer\n");
    free(istat);
    return 0;
}
