// multimod_mic.c
// Date Created: 2023-07-25
// Date Updated: 2024-08-01
// Defines for mic functions

/************************************Includes***************************************/

#include "../multimod_mic.h"

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/i2c.h"
#include "driverlib/ssi.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/adc.h"

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// MutimodMic_Init
// Initializes ports & adc module for mic
// Return: void
void MultimodMic_Init(void) {

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlDelay(3);

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1); // PE1/AIN2
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_MOSC | ADC_CLOCK_RATE_FULL, 1);
    ADCHardwareOversampleConfigure(ADC0_BASE, 64); // Enable hardware averaging with 64 samples
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_TIMER, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH2 | ADC_CTL_IE | ADC_CTL_END);

    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/(MIC_SAMPLE_RATE_HZ));
    TimerEnable(TIMER0_BASE, TIMER_A);
    TimerControlTrigger(TIMER0_BASE,TIMER_A,true);

    ADCSequenceEnable(ADC0_BASE, 1);
    ADCIntClear(ADC0_BASE, 1);
    ADCIntEnable(ADC0_BASE, 1);
}

/********************************Public Functions***********************************/
