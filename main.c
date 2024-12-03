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
    // Sets clock speed to 80 MHz. You'll need it!
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    // you might want a delay here (~10 ms) to make sure the display has powered up
    delay_ms(10);
    // Disable interrupts
    IntMasterDisable();
    // initialize the G8RTOS framework
    G8RTOS_Init();
    multimod_init();

    Threads_Init();

    // make sure screen is cleared
    ST7789_Fill(ST7789_BLACK);

    IntMasterEnable();

    G8RTOS_Launch();
    while (1);
}

/************************************MAIN*******************************************/
