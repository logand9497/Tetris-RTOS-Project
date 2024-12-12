// Final Project, uP2 Fall 2024
// Created: 2024-11-20
// Updated: 2024-11-20
// This final project is implementing the game Tetris

/************************************Includes***************************************/

#include "G8RTOS/G8RTOS.h"
#include "./MultimodDrivers/multimod.h"

#include "./threads.h"
#include "driverlib/interrupt.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/
/*************************************Defines***************************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/


/************************************MAIN*******************************************/

int main(void)
{
    // Sets clock speed to 80 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    // delay here (~10 ms) to make sure the display has powered up
    delay_ms(10);
    // Disable interrupts
    IntMasterDisable();
    // initialize the G8RTOS framework
    G8RTOS_Init();
    // initialize multimod functions
    multimod_init();

    // initialize all threads
    Threads_Init();

    IntMasterEnable();
    G8RTOS_Launch();
    while (1);
}

/************************************MAIN*******************************************/
