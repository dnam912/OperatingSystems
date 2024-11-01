// GLOBAL INCLUDE FILES
#include <iostream>
#include <queue>
#include <mutex>

// LINUX HEADER FILES
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
// #include <condition_variable> // Add

#include <unistd.h>    // Needed for fork
#include <sys/wait.h>  // Needed for wait

using namespace std;


// BOUNDED BUFFER
const char* SHARED_MEM_NAME = "/my_shared_memory";
const int BUFFER_SIZE = 2; // The table can only hold two items at the same time.

// Assign 1 to one of parameters named 'pshared' when initializing semaphores
int pshared = 1;

// INITIALIZE MAP FOR A BUFFER IN A SHARED MEMORY
struct SharedMemory {
    int buffer[BUFFER_SIZE];
    int index_counter;
    int consumer_out; // Items for consumer

    int producer_in;

    // INITIALIZE SEMAPHORES
    sem_t mutex, full, empty;
};

int item;

void consume() { 
    // CREATE SHARED MEMORY OBJECT
    // int shm_open(const char *name, int oflag, mode_t mode);
    int shm_object = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
    
    // DECIDE THE SIZE OF SHARED MEM
    ftruncate(shm_object, 1024);

    // Map the SHARED_MEM into the process's address space
    SharedMemory* SHARED_MEM = (SharedMemory*)mmap(0, sizeof(SharedMemory),
                                PROT_READ | PROT_WRITE, MAP_SHARED, shm_object, 0);

    // Assign zero in variables of SHARED MEM
    SHARED_MEM->index_counter = 0;
    SHARED_MEM->producer_in = 0; // Produce an item in the buffer

    // INITIALIZE SEMAPHORES: sem_init(sem_t *sem, int pshared, unsigned value);
    sem_init(&SHARED_MEM->mutex, pshared, 1);           // sem_mutex initialized to value 1 (binary semaphore)
    sem_init(&SHARED_MEM->full, pshared, 0);            // sem_full initailzied to value 0 (counting semaphore)
    sem_init(&SHARED_MEM->empty, pshared, BUFFER_SIZE); // sem_empty initialized to value n (counting sempaphore)
    

    // PRODUCER
    for (int i = 0; i < BUFFER_SIZE; i++) {
        cout << "DEBUG_1: Assign i into 'buffer[producer_in]" << endl;
        SHARED_MEM->buffer[SHARED_MEM->producer_in] = i;

        cout << "Produced: " << SHARED_MEM->buffer[SHARED_MEM->producer_in] << endl;
            
        SHARED_MEM->index_counter++; // Increment counter of current buffer index
    }
    
    // CONSUMER
    // for (int i = SHARED_MEM->index_counter; i >= 0; i++) {
    for (int i = BUFFER_SIZE; i >= 0; i--) {
        // item = SHARED_MEM->buffer[SHARED_MEM->consumer_out];

        item = SHARED_MEM->buffer[SHARED_MEM->index_counter];
        cout << "Consumed: " << item << endl;

        SHARED_MEM->index_counter--;
    }

    // DESTROY ASSIGNED SEMAPHORES
    sem_destroy(&SHARED_MEM->mutex);
    sem_destroy(&SHARED_MEM->full);
    sem_destroy(&SHARED_MEM->empty);

    // REMOVE THE MAP CONTAINING THE ADDRESS SPACE OF PROCESS
    munmap(&SHARED_MEM, sizeof(SharedMemory)); 

    // UNLINK SHARED MEMORY OBJECT
    // int shm_unlink(const char *name);
    shm_unlink(SHARED_MEM_NAME);
}

    /*do {  
            wait(full); // wait until full > 0 and decrement full
            wait(mutex); // acquire lock
            
            // remove an item

            signal(mutex); // release lock
            signal(empty); // increment empty
            
            // consume data from buffer
            

        } while (true); */

int main() {
    consume();
    /*
    // Fork to create processes (producer & consumer)
    pid_t pid = fork();
        PROCESSES
            (pid)
            /   \
    (producer)  (consumer)
    
    if (pid == 0) {
        consume();
        return 0;
    } else if (pid > 0) {
        wait(NULL);
    } else {
        cerr << "Fork failed" << endl;
        return 1;
    }*/

    return 0;
}
