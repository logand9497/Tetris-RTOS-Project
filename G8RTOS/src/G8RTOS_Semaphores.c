// G8RTOS_Semaphores.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for semaphore functions

#include "../G8RTOS_Semaphores.h"

/************************************Includes***************************************/

#include "../G8RTOS_CriticalSection.h"
#include "../G8RTOS_Scheduler.h"

#include "inc/hw_types.h"
#include "inc/hw_nvic.h"

/************************************Includes***************************************/

/******************************Data Type Definitions********************************/
/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/
/****************************Data Structure Definitions*****************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

// G8RTOS_InitSemaphore
// Initializes semaphore to a value.
// Param "s": Pointer to semaphore
// Param "value": Value to initialize semaphore to
// Return: void
void G8RTOS_InitSemaphore(semaphore_t* s, int32_t value) {
    IBit_State = StartCriticalSection();
    (*s) = value;
    EndCriticalSection(IBit_State);
}

// G8RTOS_WaitSemaphore
// Waits on the semaphore to become available, decrements value by 1.
// If the current resource is not available, block the current thread
// and trigger a context switch.
// Param "s": Pointer to semaphore
// Return: void
void G8RTOS_WaitSemaphore(semaphore_t* s) {
    // add your code
    IBit_State = StartCriticalSection();
    (*s)--;

    if ((*s) < 0) {
        CurrentlyRunningThread->blocked = s;
        EndCriticalSection(IBit_State);
        // yield
        HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
    }

    EndCriticalSection(IBit_State);
}

// G8RTOS_SignalSemaphore
// Signals that the semaphore has been released by incrementing the value by 1.
// Unblocks all threads currently blocked on the semaphore.
// Param "s": Pointer to semaphore
// Return: void
void G8RTOS_SignalSemaphore(semaphore_t* s) {
    IBit_State = StartCriticalSection();
    (*s)++;

    if ((*s) <= 0) {
        tcb_t * CurrentlyConsideredThread = CurrentlyRunningThread->nextTCB;

        while (CurrentlyConsideredThread->blocked != s) {
            CurrentlyConsideredThread = CurrentlyConsideredThread->nextTCB;
        }

        CurrentlyConsideredThread->blocked = 0;
    }
    EndCriticalSection(IBit_State);
}

/********************************Public Functions***********************************/
