// G8RTOS_Threads.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for thread functions.

/************************************Includes***************************************/

#include "./threads.h"

#include "./MultimodDrivers/multimod.h"

#include "./G8RTOS/G8RTOS_IPC.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "driverlib/timer.h"
#include "driverlib/adc.h"


/*********************************Global Variables**********************************/

/*********************************Global Variables**********************************/

/********************************Public Functions***********************************/

/*************************************Threads***************************************/

void Idle_Thread(void) {
    while(1);
}

void Display_Thread(void) {

    // Initialize / declare any variables here

    while(1) {
    }
}

void Read_Buttons(void) {

    // Initialize / declare any variables here
    uint8_t buttons;

    while(1) {
        // wait for button semaphore
        G8RTOS_WaitSemaphore(&sem_PCA9555_Debounce);
        // debounce buttons
        sleep(15);
        // Get buttons
        buttons = MultimodButtons_Get();
        // clear button interrupt
        GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        // update current_buttons value
        G8RTOS_WriteFIFO(BUTTONS_FIFO, buttons);
    }
}


/********************************Periodic Threads***********************************/


/*******************************Aperiodic Threads***********************************/

void Button_Handler() {
    // disable interrupt and signal semaphore
    GPIOIntDisable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
    G8RTOS_SignalSemaphore(&sem_PCA9555_Debounce);
}
