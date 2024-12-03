// G8RTOS_IPC.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for FIFO functions for interprocess communication

#include "../G8RTOS_IPC.h"

/************************************Includes***************************************/

#include "../G8RTOS_Semaphores.h"

/************************************Includes***************************************/

/******************************Data Type Definitions********************************/

#define FIFO_SIZE 16
#define MAX_NUMBER_OF_FIFOS 4

/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/

typedef struct G8RTOS_FIFO_t {
    uint32_t buffer[FIFO_SIZE];
    uint32_t *head;
    uint32_t *tail;
    uint32_t lost_data;
    semaphore_t currentSize;
    semaphore_t mutex;
} G8RTOS_FIFO_t;

/****************************Data Structure Definitions*****************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/********************************Public Variables***********************************/

static G8RTOS_FIFO_t FIFOs[MAX_NUMBER_OF_FIFOS];

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

// G8RTOS_InitFIFO
// Initializes FIFO, points head & tai to relevant buffer
// memory addresses.
// Param "FIFO_index": Index of FIFO block
// Return: int32_t, -1 if error (i.e. FIFO full), 0 if okay
int32_t G8RTOS_InitFIFO(uint32_t FIFO_index) {
    if (FIFO_index >= MAX_NUMBER_OF_FIFOS) {
        return -1;
    } else {
        FIFOs[FIFO_index].head = &(FIFOs[FIFO_index].buffer[0]);
        FIFOs[FIFO_index].tail = &(FIFOs[FIFO_index].buffer[0]);
        FIFOs[FIFO_index].lost_data = 0;
        G8RTOS_InitSemaphore(&FIFOs[FIFO_index].currentSize, 0);
        G8RTOS_InitSemaphore(&FIFOs[FIFO_index].mutex, 1);

        return 0;
    }
}

// G8RTOS_ReadFIFO
// Reads data from head pointer of FIFO.
// Param "FIFO_index": Index of FIFO block
// Return: int32_t, data at head pointer
int32_t G8RTOS_ReadFIFO(uint32_t FIFO_index) {
    if (FIFO_index >= MAX_NUMBER_OF_FIFOS) {
        return INT32_MAX;
    } else {
        // Wait if mutex is locked
        G8RTOS_WaitSemaphore(&FIFOs[FIFO_index].mutex);
        // Wait if there is no data
        G8RTOS_WaitSemaphore(&FIFOs[FIFO_index].currentSize);

        // Get the data stored in FIFO head
        int32_t data = *FIFOs[FIFO_index].head;

        // Increment head pointer
        FIFOs[FIFO_index].head++;

        // Wrap around if needed
        if (FIFOs[FIFO_index].head > &(FIFOs[FIFO_index].buffer[0]) + (FIFO_SIZE-1)) {
            FIFOs[FIFO_index].head = &(FIFOs[FIFO_index].buffer[0]);
        }

        // Release mutex
        G8RTOS_SignalSemaphore(&FIFOs[FIFO_index].mutex);

        return data;
    }
}

// G8RTOS_WriteFIFO
// Writes data to tail of buffer.
// Param "FIFO_index": Index of FIFO block
// Return: int32_t, data at head pointer
int32_t G8RTOS_WriteFIFO(uint32_t FIFO_index, uint32_t data) {
    // If index is out of range, return -1
    if (FIFO_index >= MAX_NUMBER_OF_FIFOS) {
        return -1;
    }

    // If the current size of the FIFO is greater than the max FIFO
    // size, the data is lost and return -2.
    if (FIFOs[FIFO_index].currentSize >= FIFO_SIZE) {
        *FIFOs[FIFO_index].tail = data;
        FIFOs[FIFO_index].lost_data++;
        return -2;
    }

    // Set the tail pointer to the data
    *FIFOs[FIFO_index].tail = data;

    // Increment tail, wrap around if needed
    FIFOs[FIFO_index].tail++;
    if (FIFOs[FIFO_index].tail > &(FIFOs[FIFO_index].buffer[FIFO_SIZE-1])) {
        FIFOs[FIFO_index].tail = &(FIFOs[FIFO_index].buffer[0]);
    }

    // Increase current size
    G8RTOS_SignalSemaphore(&(FIFOs[FIFO_index].currentSize));

    return 0;
}

/********************************Public Functions***********************************/
