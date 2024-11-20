// threads.h
// Date Created: 2023-07-26
// Date Updated: 2023-07-26
// Threads

#ifndef THREADS_H_
#define THREADS_H_

/************************************Includes***************************************/

#include "./G8RTOS/G8RTOS.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define BUTTONS_FIFO        0
#define JOYSTICK_FIFO       1
#define DISPLAY_FIFO        2

/*************************************Defines***************************************/

/***********************************Semaphores**************************************/

semaphore_t sem_I2CA;
semaphore_t sem_SPIA;
semaphore_t sem_PCA9555_Debounce;
semaphore_t sem_Joystick_Debounce;

/***********************************Semaphores**************************************/

/***********************************Structures**************************************/
/***********************************Structures**************************************/


/*******************************Background Threads**********************************/

void Idle_Thread(void);
void Display_Thread(void);
void Read_Buttons(void);

/*******************************Background Threads**********************************/

/********************************Periodic Threads***********************************/

/********************************Periodic Threads***********************************/

/*******************************Aperiodic Threads***********************************/

void Button_Handler(void);

/*******************************Aperiodic Threads***********************************/


#endif /* THREADS_H_ */

