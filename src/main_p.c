#include "main_p.h"
#include "../lib/reader/reader.h"
#include "../lib/buffer/buffer.h"
#include "../lib/analyzer/analyzer.h"
#include "../lib/printer/printer.h"
#include "../lib/logger/logger.h"
#include "../lib/watchdog/watchdog.h"

//EXTERNAL VARIABLES
sem_t empty, full, empty_r, full_r;                     //semaphores
pthread_mutex_t conch, conch_r;                         //mutexes
pthread_t reader, analyzer, printer, logger, watchdog;  //thread descriptors
volatile sig_atomic_t w;                                //watchdog and main atomic
volatile sig_atomic_t t;                                //procedures atomic
/*******************/

//INTERNAL VARIABLES
static ring_buffer* stat_buffer;                        //buffer for stats
static ring_buffer* res_buffer;                         //buffer for results
static struct sigaction actionTERM;                     //decalre sigaction structure (for SIGTERM)
static struct sigaction actionINT;                      //decalre sigaction structure (for SIGTERM)
/*******************/

/// @brief Unlocks memory from globals used in program
void destroy_leftovers(void)
{
    log_line("In destroy_leftovers");
    //destroy semaphores and mutex
    log_line("stat_buffer access control destroy");
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&conch);

    log_line("res_buffer access control destroy");
    sem_destroy(&empty_r);
    sem_destroy(&full_r);
    pthread_mutex_destroy(&conch_r);

    log_line("freeing buffer memory");
    //free alocated memory
    ring_buffer_free(stat_buffer);
    ring_buffer_free(res_buffer);
}

/// @brief Initializes globals used in the program
void initialize_program_variables(void)
{
    log_line("Initializing atomic variables");
    w = 1;                                                      //driver variables
    t = 1;
    printf("Enetring initialization process\n");
    
    log_line("Setting a SIGTERM sigaction");
    memset(&actionTERM, 0, sizeof(struct sigaction));           //clear the sigaction structure memory
    actionTERM.sa_handler = terminate;                          //assign a function to this action handler
    sigaction(SIGTERM, &actionTERM, NULL);                      //assign a signal value to the action structure

    log_line("Setting a SIGINT sigaction");
    memset(&actionINT, 0, sizeof(struct sigaction));            //clear the sigaction structure memory
    actionINT.sa_handler = terminate;                           //assign a function to this action handler
    sigaction(SIGINT, &actionINT, NULL);                        //assign a signal value to the action structure
    
    //buffers initialization
    printf("Allocating the buffer\n");
    log_line("stat_buffer alocating memory");
    stat_buffer = (ring_buffer*)malloc(sizeof(ring_buffer));    //memmory allocation
    if(stat_buffer == NULL)                                     //check if the memmory was alocated
    {
        log_line("stat_buffer was unable to allocate memory. Out of heap memory?");
        perror("NULL exception in malloc\n");
    }
    log_line("Initializing stat_buffer");
    stats_ring_buffer_init(stat_buffer);                        //initialize stats ring buffer

    log_line("res_buffer alocating memory");
    res_buffer = (ring_buffer*)malloc(sizeof(ring_buffer));     //memmory allocation
    if(res_buffer == NULL)                                      //check if the memory was alocated
    {
        log_line("res_buffer was unable to allocate memory. Out of heap memory?");
        perror("NULL exception in malloc\n");
    }
    log_line("Initializing res_buffer");
    res_ring_buffer_init(res_buffer);                           //initialize results ring buffer
    //buffers initialization end
    
    log_line("Initilize stat_buffer access control variables");
    //stats_cpu buffer sem and mutex
    sem_init(&empty, 0, BUFFER_SIZE - 1);                       //initialize the semaphore variable (empty slots in buffer)
    sem_init(&full, 0, 0);                                      //initialize the semaphore variable (full slots in buffer)
    pthread_mutex_init(&conch, NULL);                           //initialize the mutex

    //results buffer sem and mutex
    log_line("Initialize res_buffer access controle variables");
    sem_init(&empty_r, 0, BUFFER_SIZE - 1);                     //initialize the semaphore variable (empty slots in buffer)
    sem_init(&full_r, 0, 0);                                    //initialize the semaphore variable (full slots in buffer)
    pthread_mutex_init(&conch_r, NULL);                         //initialize the mutex
}

/// @brief Reader procedure - starts reading of the /proc/stat file, returns results to stats buffer and oversess access to stats buffer
/// @param s pointer of pthread_t
/// @return s pointer to pthread_join
void* reader_procedure(void *s)
{   
    log_line("Starting reader procedure");
    reader_init();
    reader_main(stat_buffer);
    reader_destroy();
    return s;
}


/// @brief Analyzer procedure - starts stats computations and oversses the buffer's access
/// @param s pointer of pthread_t
/// @return s pointer to pthread_join
void* analyze_stats (void *s)
{   
    log_line("Starting analyzer procedure");
    analyzer_init();
    analyzer_main(stat_buffer, res_buffer);
    analyzer_destroy();
    return s;
}

/// @brief Printer procedure - prints the values of percentage for each CPU to the console
/// @param s pointer of pthread_t
/// @return s pointer to pthread_join
void* print_stats(void *s)
{
    log_line("Starting printer procedure");
    printer_init();
    printer_main(res_buffer);
    printer_destroy();
    return s;
}

/// @brief Watchdog procedure - terminates execution of the program if one of threads times out
/// @param s pointer of pthread_t
/// @return s pointer to pthread_join
void* watchdog_watch(void *s)
{
    log_line("Starting main loop of the watchdog");
    watchdog_main();
    return s;
}

