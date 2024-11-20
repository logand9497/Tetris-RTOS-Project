// G8RTOS_Scheduler.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for scheduler functions

#include "../G8RTOS_Scheduler.h"

/************************************Includes***************************************/

#include <stdint.h>
#include <stdbool.h>

#include "../G8RTOS_CriticalSection.h"

#include <inc/hw_memmap.h>
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"

/************************************Includes***************************************/

/********************************Private Variables**********************************/

// Thread Control Blocks - array to hold information for each thread
static tcb_t threadControlBlocks[MAX_THREADS];

// Thread Stacks - array of arrays for individual stacks of each thread
static uint32_t threadStacks[MAX_THREADS][STACKSIZE];

// Periodic Event Threads - array to hold pertinent information for each thread
static ptcb_t pthreadControlBlocks[MAX_PTHREADS];

// Current Number of Threads currently in the scheduler
static uint32_t NumberOfThreads;

// Current Number of Periodic Threads currently in the scheduler
static uint32_t NumberOfPThreads;

static uint32_t threadCounter = 0;

/********************************Private Variables**********************************/

/*******************************Private Functions***********************************/

// Occurs every 1 ms.
static void InitSysTick(void)
{
    SysTickPeriodSet(SysCtlClockGet() / 1000);
    SysTickIntRegister(SysTick_Handler);
    IntRegister(FAULT_PENDSV, PendSV_Handler);
    SysTickIntEnable();
    SysTickEnable();
}

/*******************************Private Functions***********************************/


/********************************Public Variables***********************************/

uint32_t SystemTime;

tcb_t* CurrentlyRunningThread;

/********************************Public Variables***********************************/



/********************************Public Functions***********************************/

// SysTick_Handler
// Increments system time, sets PendSV flag to start scheduler.
// Return: void
void SysTick_Handler() {
    SystemTime++;

    tcb_t* currThread = CurrentlyRunningThread;

    int i = 0;
    for (; i < NumberOfThreads; i++) {
        if (currThread->sleepCount <= SystemTime) {
            currThread->asleep = 0;
        }
        currThread = currThread->nextTCB;
    }

    i = 0;
    for (; i < NumberOfPThreads; i++) {
        if (pthreadControlBlocks[i].executeTime <= SystemTime) {
            pthreadControlBlocks[i].handler();
            pthreadControlBlocks[i].executeTime = SystemTime + pthreadControlBlocks[i].period;
        }
    }

    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
}

// G8RTOS_Init
// Initializes the RTOS by initializing system time.
// Return: void
void G8RTOS_Init() {
    uint32_t newVTORTable = 0x20000000;
    uint32_t* newTable = (uint32_t*) newVTORTable;
    uint32_t* oldTable = (uint32_t*) 0;

    int i = 0;
    for (; i < 155; i++) {
        newTable[i] = oldTable[i];
    }

    HWREG(NVIC_VTABLE) = newVTORTable;

    SystemTime = 0;
    NumberOfThreads = 0;
    NumberOfPThreads = 0;
}

// G8RTOS_Launch
// Launches the RTOS.
// Return: error codes, 0 if none
int32_t G8RTOS_Launch() {
    InitSysTick();

    CurrentlyRunningThread = &threadControlBlocks[0];
    IntPrioritySet(FAULT_SYSTICK, 0xE0);
    IntPrioritySet(FAULT_PENDSV, 0xE0);
    G8RTOS_Start(); // call the assembly function
    return 0;
}

// G8RTOS_Scheduler
// Chooses next thread in the TCB. This time uses priority scheduling.
// Return: void
void G8RTOS_Scheduler() {
    uint16_t next_thread_priority = UINT8_MAX + 1;
    tcb_t* next_thread = CurrentlyRunningThread->nextTCB;

    int i = 0;
    for (; i < NumberOfThreads; i++) {
        if (next_thread->blocked == 0 && next_thread->asleep == 0) {
            if (next_thread->priority < next_thread_priority) {
                CurrentlyRunningThread = next_thread;
                next_thread_priority = CurrentlyRunningThread->priority;
            }
        }
        next_thread = next_thread->nextTCB;
    }
}

// G8RTOS_AddThread
// Adds a thread. This is now in a critical section to support dynamic threads.
// It also now should initalize priority and account for live or dead threads.
// Param void* "threadToAdd": pointer to thread function address
// Param uint8_t "priority": priority from [0, 255].
// Param char* "name": character array containing the thread name.
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_AddThread(void (*threadToAdd)(void), uint8_t priority, char *name) {
    IBit_State = StartCriticalSection();

    uint8_t i = 0;
    uint8_t j = 0;

    if (G8RTOS_GetNumberOfThreads() >= MAX_THREADS) {
            EndCriticalSection(IBit_State);
            return THREAD_LIMIT_REACHED;
    } else {
        if (G8RTOS_GetNumberOfThreads() == 0) {
            threadControlBlocks[0].nextTCB = &(threadControlBlocks[0]);
            threadControlBlocks[0].previousTCB = &(threadControlBlocks[0]);

        } else {

            while (threadControlBlocks[i].isAlive == 1) {
                i++;
            }

            if (i == 0) {
                j = MAX_THREADS - 1;
            } else {
                j = i - 1;
            }

            while (threadControlBlocks[j].isAlive == 0) {
                j--;
            }

            threadControlBlocks[i].nextTCB = threadControlBlocks[j].nextTCB;
            threadControlBlocks[i].previousTCB = &(threadControlBlocks[j]);
            threadControlBlocks[j].nextTCB->previousTCB = &(threadControlBlocks[i]);
            threadControlBlocks[j].nextTCB = &(threadControlBlocks[i]);

        }

        threadControlBlocks[i].ThreadID = threadCounter++;
        threadControlBlocks[i].asleep = 0;
        threadControlBlocks[i].blocked = 0;
        threadControlBlocks[i].sleepCount = 0;
        threadControlBlocks[i].priority = priority;
        threadControlBlocks[i].isAlive = 1;

        j = 0;
        while (name[j] != '\0' && j < MAX_NAME_LENGTH - 1) {
            threadControlBlocks[i].threadName[j] = name[j];
            j++;
        }
        name[j] = '\0';

        // Set up the stack pointer
        threadControlBlocks[i].stackPointer = &threadStacks[i][STACKSIZE - 50];
        threadStacks[i][STACKSIZE - 19] = THUMBBIT;
        threadStacks[i][STACKSIZE - 20] = (uint32_t)threadToAdd; // address to start of function

        // Increment number of threads present in the scheduler
        NumberOfThreads++;
    }

    EndCriticalSection(IBit_State);
    return NO_ERROR;
}

// G8RTOS_Add_APeriodicEvent
// Param void* "AthreadToAdd": pointer to thread function address
// Param uint8_t "priority": Priorit of aperiodic event, [1..6]
// Param int32_t "IRQn": Interrupt request number that references the vector table. [0..155].
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_Add_APeriodicEvent(void (*AthreadToAdd)(void), uint8_t priority, int32_t IRQn) {
    IBit_State = StartCriticalSection();            //Disable interrupts

    if (IRQn < 0 || IRQn > 155) {
        EndCriticalSection(IBit_State);             //Enables interrupts
        return IRQn_INVALID;
    }

    if (priority > 6 || priority < 1) {               //Checks if priority too low
        EndCriticalSection(IBit_State);             //Enables interrupts
        return HWI_PRIORITY_INVALID;
    }

    uint32_t *vectors = (uint32_t *)HWREG(NVIC_VTABLE);
    vectors[IRQn] = (uint32_t)AthreadToAdd;

    IntPrioritySet(IRQn, priority);
    IntEnable(IRQn);
    EndCriticalSection(IBit_State);
    return NO_ERROR;
}

// G8RTOS_Add_PeriodicEvent
// Adds periodic threads to G8RTOS Scheduler
// Function will initialize a periodic event struct to represent event.
// The struct will be added to a linked list of periodic events
// Param void* "PThreadToAdd": void-void function for P thread handler
// Param uint32_t "period": period of P thread to add
// Param uint32_t "execution": When to execute the periodic thread
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_Add_PeriodicEvent(void (*PThreadToAdd)(void), uint32_t period, uint32_t execution) {
    // your code
    if (NumberOfPThreads > MAX_PTHREADS) {
        return THREAD_LIMIT_REACHED;
    } else {
        if (NumberOfPThreads == 0) {
            pthreadControlBlocks[0].nextPTCB = &(pthreadControlBlocks[0]);
            pthreadControlBlocks[0].previousPTCB = &(pthreadControlBlocks[0]);
        } else {
            pthreadControlBlocks[NumberOfPThreads].nextPTCB = &pthreadControlBlocks[0];
            pthreadControlBlocks[0].previousPTCB = &pthreadControlBlocks[NumberOfPThreads];

            pthreadControlBlocks[NumberOfPThreads].previousPTCB = &pthreadControlBlocks[NumberOfPThreads-1];
            pthreadControlBlocks[NumberOfPThreads-1].nextPTCB = &pthreadControlBlocks[NumberOfPThreads];
        }

        pthreadControlBlocks[NumberOfPThreads].handler = PThreadToAdd;
        pthreadControlBlocks[NumberOfPThreads].period = period;
        pthreadControlBlocks[NumberOfPThreads].executeTime = execution;

        // Increment number of threads present in the scheduler
        NumberOfPThreads++;
    }

    return NO_ERROR;
}

// G8RTOS_KillThread
// Param uint32_t "threadID": ID of thread to kill
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_KillThread(threadID_t threadID) {
    IBit_State = StartCriticalSection();
    // Loop through tcb. If not found, return thread does not exist error. If there is only one thread running, don't kill it.
    if (G8RTOS_GetNumberOfThreads() == 1) {
        EndCriticalSection(IBit_State);
        return CANNOT_KILL_LAST_THREAD;
    } else {
        tcb_t* currThread = CurrentlyRunningThread;

        if (currThread->ThreadID == threadID) {
            currThread->isAlive = 0;

            currThread->previousTCB->nextTCB = currThread->nextTCB;
            currThread->nextTCB->previousTCB = currThread->previousTCB;

        } else {
            currThread = currThread->nextTCB;
            while(currThread->ThreadID != threadID) {
                currThread = currThread->nextTCB;
                if (currThread = CurrentlyRunningThread) {
                    EndCriticalSection(IBit_State);
                    return THREAD_DOES_NOT_EXIST;
                }
            }

            currThread->isAlive = 0;

            currThread->previousTCB->nextTCB = currThread->nextTCB;
            currThread->nextTCB->previousTCB = currThread->previousTCB;
        }

        if (currThread->blocked) {
            *(currThread->blocked)++;
        }

        NumberOfThreads--;

        EndCriticalSection(IBit_State);

        return NO_ERROR;
    }
}

// G8RTOS_KillSelf
// Kills currently running thread.
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_KillSelf() {
    // your code
    IBit_State = StartCriticalSection();

    if (G8RTOS_GetNumberOfThreads() == 1) {
        EndCriticalSection(IBit_State);
        return CANNOT_KILL_LAST_THREAD;
    }

    NumberOfThreads--;

    CurrentlyRunningThread->isAlive = 0;
    CurrentlyRunningThread->previousTCB->nextTCB = CurrentlyRunningThread->nextTCB;
    CurrentlyRunningThread->nextTCB->previousTCB = CurrentlyRunningThread->previousTCB;

    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
    EndCriticalSection(IBit_State);

    return NO_ERROR;
}

// sleep
// Puts current thread to sleep
// Param uint32_t "durationMS": how many systicks to sleep for
void sleep(uint32_t durationMS) {
    CurrentlyRunningThread->sleepCount = durationMS + SystemTime;
    CurrentlyRunningThread->asleep = 1;
    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
}

// G8RTOS_GetThreadID
// Gets current thread ID.
// Return: threadID_t
threadID_t G8RTOS_GetThreadID(void) {
    return CurrentlyRunningThread->ThreadID;        //Returns the thread ID
}

// G8RTOS_GetNumberOfThreads
// Gets number of threads.
// Return: uint32_t
uint32_t G8RTOS_GetNumberOfThreads(void) {
    return NumberOfThreads;         //Returns the number of threads
}
/********************************Public Functions***********************************/
