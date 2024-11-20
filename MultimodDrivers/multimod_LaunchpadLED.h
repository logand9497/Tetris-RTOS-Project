// multimod_LaunchpadButtons.h
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// header file for functions related to buttons on the launchpad

#ifndef MULTIMOD_LAUNCHPADLED_H_
#define MULTIMOD_LAUNCHPADLED_H_

/************************************Includes***************************************/

#include <stdint.h>
#include <stdbool.h>

#include <driverlib/gpio.h>

/************************************Includes***************************************/

/*************************************Defines***************************************/

// You can adjust this for "finer" control over the duty cycle.
#define PWM_Per                     400

/*************************************Defines***************************************/

/******************************Data Type Definitions********************************/
/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/

typedef enum {
    RED = GPIO_PIN_1,
    BLUE = GPIO_PIN_2,
    GREEN = GPIO_PIN_3
} LED_Color_t;

/****************************Data Structure Definitions*****************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

void LaunchpadLED_Init();
void LaunchpadLED_PWMSetDuty(LED_Color_t LED, float duty);

/********************************Public Functions***********************************/

/*******************************Private Variables***********************************/
/*******************************Private Variables***********************************/

/*******************************Private Functions***********************************/
/*******************************Private Functions***********************************/

#endif /* MULTIMOD_LAUNCHPADLED_H_ */


