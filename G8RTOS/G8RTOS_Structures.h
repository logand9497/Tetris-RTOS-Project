// G8RTOS_Structures.h
// Date Created: 2023-07-26
// Date Updated: 2023-07-26
// Thread block definitions

#ifndef G8RTOS_STRUCTURES_H_
#define G8RTOS_STRUCTURES_H_

/************************************Includes***************************************/

#include <stdbool.h>
#include <stdint.h>

#include "G8RTOS_Structures.h"
#include "G8RTOS_Semaphores.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define MAX_NAME_LENGTH             16

/*************************************Defines***************************************/

/******************************Data Type Definitions********************************/

// Thread ID
typedef int32_t threadID_t;

/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/

// Thread Control Block
typedef struct tcb_t {
    uint32_t *stackPointer;
    struct tcb_t *nextTCB;
    struct tcb_t *previousTCB;
    semaphore_t *blocked;
    uint32_t sleepCount;
    bool asleep;
    uint8_t priority;
    bool isAlive;
    char threadName[MAX_NAME_LENGTH];
    threadID_t ThreadID;
} tcb_t;

// Periodic Thread Control Block
typedef struct ptcb_t {
    void (*handler)(void);
    struct ptcb_t *previousPTCB;
    struct ptcb_t *nextPTCB;
    uint32_t period;
    uint32_t executeTime;
    uint32_t currentTime;
} ptcb_t;

/****************************Data Structure Definitions*****************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/

/********************************Public Functions***********************************/
/********************************Public Functions***********************************/

/*******************************Private Variables***********************************/
/*******************************Private Variables***********************************/

/*******************************Private Functions***********************************/
/*******************************Private Functions***********************************/

#endif /* FILENAME_H_ */
